#include "circularbufferplayer.h"


RTTI_BEGIN_CLASS(nap::audio::CircularBufferPlayer)
    RTTI_PROPERTY("ChannelCount", &nap::audio::CircularBufferPlayer::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        SafeOwner<Node> CircularBufferPlayer::createNode(int channel, AudioService& audioService, utility::ErrorState& errorState)
        {
            auto node = audioService.makeSafe<CircularBufferPlayerNode>(audioService.getNodeManager());
            return std::move(node);
        }
                
    }
    
}
