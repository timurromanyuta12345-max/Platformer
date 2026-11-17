#pragma once
#include "Anim_Base.h"

class Anim_Directional : public Anim_Base {
protected:
	virtual void frameStep() override;
	virtual void cropSite() override;
	virtual void readIn(std::spanstream& l_stream) override;
	virtual size_t getSize() const noexcept override;
};