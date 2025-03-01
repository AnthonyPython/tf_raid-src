//=============================================================================//
//
// Purpose: Arms that get bonemerged with a viewmodel
// Originally written for PF2
// Noodles;
// Date created: 25/10/2021
// Last modifed: 5/02/2023
//
//=============================================================================//

#include "cbase.h"
#include "c_baseplayer.h"
#include "hdtf_viewmodel_arms.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_HDTFArms *C_HDTFArms::CreateViewModelArms( C_BaseEntity *pParent, const char* modelname )
{
	C_HDTFArms *pArms = new C_HDTFArms();
	if ( !pArms)
		return NULL;

	if ( !pArms->InitializeVMArms( pParent, modelname) )
		return NULL;

	return pArms;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_HDTFArms::InitializeVMArms( C_BaseEntity *pParent, const char* modelname )
{
	if(!modelname)
		return false;

	C_BaseViewModel *pViewModel = dynamic_cast<C_BaseViewModel *>(pParent);

	AddEffects( EF_BONEMERGE | EF_BONEMERGE_FASTCULL | EF_PARENT_ANIMATES );
	if ( !pViewModel || !InitializeAsClientEntity(modelname, RENDER_GROUP_VIEW_MODEL_TRANSLUCENT ) )
	{
		Release();
		return false;
	}
	
	m_pParent = pViewModel;
	SetParent( pViewModel );
	SetLocalOrigin( vec3_origin );
	//FollowEntity( pViewModel, true );

	AddSolidFlags( FSOLID_NOT_SOLID );
	SetNextClientThink( CLIENT_THINK_ALWAYS );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : RenderGroup_t
//-----------------------------------------------------------------------------
RenderGroup_t C_HDTFArms::GetRenderGroup()
{
	return RENDER_GROUP_VIEW_MODEL_TRANSLUCENT;
}

//-----------------------------------------------------------------------------
// Purpose: Catches if they somehow get orphaned.
//			Actual fix is probably to override UnlinkChild in the viewmodel
//-----------------------------------------------------------------------------
void C_HDTFArms::ClientThink(void)
{
	if( !GetMoveParent() )
	{
		// Delete thyself immediately
		m_pParent->RemoveViewmodelArms();
		return;
	}
}