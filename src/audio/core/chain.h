// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>

namespace nap
{
    
    namespace audio
    {
        
        class NAPAPI Chain : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
        public:
            Chain() : AudioObject() { }
            
            std::vector<ResourcePtr<AudioObject>> mObjects;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };
        
        
        class NAPAPI ChainInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            ChainInstance(Chain& resource) : AudioObjectInstance(resource) { }
            bool init(AudioService& audioService, utility::ErrorState& errorState) override;
            
            OutputPin* getOutputForChannel(int channel) override { return mObjects.back()->getOutputForChannel(channel); }
            int getChannelCount() const override { return mObjects.back()->getChannelCount(); }
            
            /**
             * Use this method to acquire an object within the graph by ID.
             * @return: a pointer to the object with the given ID.
             */
            template <typename T>
            T* getObject(unsigned int index) { return rtti_cast<T>(getObjectNonTyped(index)); }
            
            /**
             * Non typed version of @getObject().
             */
            AudioObjectInstance* getObjectNonTyped(unsigned int index);
            
        private:
            virtual bool connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState);
            
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
        };
        
        
        class NAPAPI EffectChain : public Chain
        {
            RTTI_ENABLE(Chain)
            
        public:
            EffectChain() : Chain() { }
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };
        
        
        class NAPAPI EffectChainInstance : public ChainInstance, public IMultiChannelInput
        {
            RTTI_ENABLE(ChainInstance)
            
        public:
            EffectChainInstance(EffectChain& resource) : ChainInstance(resource) { }
            bool init(AudioService& audioService, utility::ErrorState& errorState) override;
            
            void connect(unsigned int channel, OutputPin& pin) override { mInput->connect(channel, pin); }
            int getInputChannelCount() const override { return mInput->getInputChannelCount(); }
            
        private:
            IMultiChannelInput* mInput = nullptr;
        };
        
        
        class NAPAPI MultiEffectChain : public EffectChain
        {
            RTTI_ENABLE(EffectChain)
        
        public:
            MultiEffectChain() : EffectChain() { }
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };
        
        
        class NAPAPI MultiEffectChainInstance : public EffectChainInstance
        {
            RTTI_ENABLE(EffectChainInstance)
        public:
            MultiEffectChainInstance(MultiEffectChain& resource) : EffectChainInstance(resource) { }
            
        private:
            virtual bool connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState);
        };
        
    }
    
}
