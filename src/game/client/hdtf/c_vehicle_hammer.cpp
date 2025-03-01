#include "cbase.h"
#include "c_prop_vehicle.h"
#include "c_vehicle_jeep.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_PropHammer : public C_PropJeep
{

	DECLARE_CLASS(C_PropHammer, C_PropJeep);

public:
	DECLARE_CLIENTCLASS();
};

IMPLEMENT_CLIENTCLASS_DT(C_PropHammer, DT_CPropHammer, CPropHammer)
END_RECV_TABLE()
