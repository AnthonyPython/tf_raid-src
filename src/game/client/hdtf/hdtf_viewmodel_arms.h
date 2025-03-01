//=============================================================================//
//
// Purpose:
//
//=============================================================================//

#ifndef HDTF_VIEWMODEL_ARMS_H
#define HDTF_VIEWMODEL_ARMS_H
#include "c_baseanimating.h"

class C_HDTFArms : public C_BaseAnimating
{
	DECLARE_CLASS(C_HDTFArms, C_BaseAnimating);
public:
	static C_HDTFArms* CreateViewModelArms(C_BaseEntity* pParent, const char* modelname);
	RenderGroup_t GetRenderGroup();
	void	ClientThink(void);

private:
	bool	InitializeVMArms(C_BaseEntity* pParent, const char* modelname);
	C_BaseViewModel* m_pParent;
};

#endif // HDTF_VIEWMODEL_ARMS_H