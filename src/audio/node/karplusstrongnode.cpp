#include "karplusstrongnode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::KarplusStrongNode)
		RTTI_PROPERTY("input", &nap::audio::KarplusStrongNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
		RTTI_PROPERTY("output", &nap::audio::KarplusStrongNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
		RTTI_FUNCTION("setDelayTime", &nap::audio::KarplusStrongNode::setDelayTime)
		RTTI_FUNCTION("setFeedback", &nap::audio::KarplusStrongNode::setFeedback)
		RTTI_FUNCTION("setDamping", &nap::audio::KarplusStrongNode::setDamping)
		RTTI_FUNCTION("setNegativePolarity", &nap::audio::KarplusStrongNode::setNegativePolarity)
RTTI_END_CLASS

