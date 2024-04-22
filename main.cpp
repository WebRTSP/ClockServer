#include <memory>

#include <CxxPtr/GlibPtr.h>
#include <CxxPtr/libwebsocketsPtr.h>

#include "Helpers/LwsLog.h"
#include "Http/Log.h"
#include "Http/HttpMicroServer.h"
#include "Signalling/Log.h"
#include "Signalling/WsServer.h"
#include "Signalling/ServerSession.h"
#include "RtStreaming/GstRtStreaming/LibGst.h"
#include "RtStreaming/GstRtStreaming/GstPipelineStreamer.h"

#define USE_HW_ENCODER 1

const char* ClockPipeline =
    "videotestsrc pattern=blue ! video/x-raw, width=640, height=480, framerate=5/1 ! "
    "clockoverlay halignment=center valignment=center shaded-background=true font-desc=\"Sans, 36\" ! "
#if USE_HW_ENCODER
    "v4l2h264enc ! video/x-h264, level=(string)4, profile=(string)constrained-baseline ! "
#else
    "x264enc ! video/x-h264, level=(string)4, profile=(string)constrained-baseline ! "
#endif
    "rtph264pay pt=99 config-interval=-1 ! webrtcbin";

static std::unique_ptr<WebRTCPeer> CreatePeer(const std::string& /*uri*/)
{
    return std::make_unique<GstPipelineStreamer>(ClockPipeline);
}

static std::unique_ptr<ServerSession> CreateSession (
    const WebRTCConfigPtr& webRTCConfig,
    const std::function<void (const rtsp::Request*)>& sendRequest,
    const std::function<void (const rtsp::Response*)>& sendResponse) noexcept
{
    return
        std::make_unique<ServerSession>(
            webRTCConfig,
            CreatePeer,
            sendRequest,
            sendResponse);
}

int main(int argc, char *argv[])
{
    LibGst libGst;

    std::shared_ptr<WebRTCConfig> webRTCConfig = std::make_shared<WebRTCConfig>();
    webRTCConfig->iceServers = WebRTCConfig::IceServers { "stun://stun.l.google.com:19302" };

    http::Config httpConfig {
        .bindToLoopbackOnly = false
    };
#ifdef SNAPCRAFT_BUILD
    const gchar* snapPath = g_getenv("SNAP");
    const gchar* snapName = g_getenv("SNAP_NAME");
    if(snapPath && snapName)
        httpConfig.wwwRoot = std::string(snapPath) + "/opt/" + snapName + "/www";
#endif
    signalling::Config config {
        .bindToLoopbackOnly = false
    };

    InitLwsLogger(spdlog::level::warn);

    GMainLoopPtr loopPtr(g_main_loop_new(nullptr, FALSE));
    GMainLoop* loop = loopPtr.get();

    lws_context_creation_info lwsInfo {};
    lwsInfo.gid = -1;
    lwsInfo.uid = -1;
    lwsInfo.options = LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
#if defined(LWS_WITH_GLIB)
    lwsInfo.options |= LWS_SERVER_OPTION_GLIB;
    lwsInfo.foreign_loops = reinterpret_cast<void**>(&loop);
#endif

    LwsContextPtr contextPtr(lws_create_context(&lwsInfo));
    lws_context* context = contextPtr.get();

    std::string configJs =
        fmt::format("const WebRTSPPort = {};\r\n", config.port);
    http::MicroServer httpServer(httpConfig, configJs, http::MicroServer::OnNewAuthToken(), g_main_context_default());
    signalling::WsServer server(
        config,
        loop,
        std::bind(
            CreateSession,
            webRTCConfig,
            std::placeholders::_1,
            std::placeholders::_2));

    if(httpServer.init() && server.init())
        g_main_loop_run(loop);
    else
        return -1;

    return 0;
}
