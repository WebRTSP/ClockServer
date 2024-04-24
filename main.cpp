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
#include "RtStreaming/GstRtStreaming/GstPipelineStreamer2.h"

const char* ClockPipeline =
    "videotestsrc pattern=blue ! "
    "clockoverlay halignment=center valignment=center shaded-background=true font-desc=\"Sans, 36\" ! "
    "x264enc ! video/x-h264, profile=baseline ! rtph264pay pt=99 config-interval=1";

static std::unique_ptr<WebRTCPeer> CreatePeer(GstPipelineStreamer2* streamer, const std::string&)
{
    return streamer->createPeer();
}

static std::unique_ptr<ServerSession> CreateSession (
    const WebRTCConfigPtr& webRTCConfig,
    GstPipelineStreamer2* streamer,
    const std::function<void (const rtsp::Request*)>& sendRequest,
    const std::function<void (const rtsp::Response*)>& sendResponse) noexcept
{
    return
        std::make_unique<ServerSession>(
            webRTCConfig,
            std::bind(CreatePeer, streamer, std::placeholders::_1),
            sendRequest,
            sendResponse);
}

int main(int argc, char *argv[])
{
    LibGst libGst;

    GstPipelineStreamer2 streamer(ClockPipeline);

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
            &streamer,
            std::placeholders::_1,
            std::placeholders::_2));

    if(httpServer.init() && server.init())
        g_main_loop_run(loop);
    else
        return -1;

    return 0;
}
