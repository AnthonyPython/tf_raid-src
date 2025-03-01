#include "cbase.h" 
#include "c_baseentity.h" // Needed so we can derive from it.
#include "engine/ivdebugoverlay.h" // Needed so there is not an undefined type error

class C_MessageEntity : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_MessageEntity, C_BaseEntity);
	DECLARE_CLIENTCLASS(); // Declare that we are a client class, and that there should be a corrispoding server class

	void ClientThink() override; // Override ClientThink so we can modify it
	void OnDataChanged(DataUpdateType_t type) override; // Same with on data changed

public:
	char m_szMessageText[128]; // Variables which will be allocated values which are being recieved from server.
	bool m_drawText; // You can modify the size of m_szMessageText on both ends to increase/decrease character count
	Vector m_vecTextOrigin;
	int m_radius;
};

LINK_ENTITY_TO_CLASS(point_message, C_MessageEntity);

IMPLEMENT_CLIENTCLASS_DT(C_MessageEntity, DT_MessageEntity, CMessageEntity) // Implement client class data table. It is important that the values are sent and recieved in the same order.

	RecvPropString(RECVINFO(m_szMessageText)),
	RecvPropBool(RECVINFO(m_drawText)),
	RecvPropVector(RECVINFO(m_vecTextOrigin)),
	RecvPropInt(RECVINFO(m_radius)),

END_RECV_TABLE()

extern IVDebugOverlay* debugoverlay; // Get and external instance of debugoverlay. Let the linker figure out where it is.

void C_MessageEntity::OnDataChanged(DataUpdateType_t type) // This is important. It is called when data is changed, or in our case as soon as we load onto the server.
{
	BaseClass::OnDataChanged(type);
	SetNextClientThink(CLIENT_THINK_ALWAYS); // We need this so we can start this entity thinking. This will make it think until the level is shutdown.
}

void C_MessageEntity::ClientThink()
{
	BaseClass::ClientThink();

	if (m_drawText) // Only draw text if enabled
	{
		if (UTIL_PlayerByIndex(GetLocalPlayerIndex())) // Get the local player
		{
			C_BasePlayer* LocalPlayer = UTIL_PlayerByIndex(GetLocalPlayerIndex()); // Assign the local player to a variable so it is easier to work with.
			Vector worldTargetPosition = LocalPlayer->EyePosition(); // Get the players position

			if ((worldTargetPosition - GetAbsOrigin()).Length() <= m_radius) // Check whether the player within range or not
			{
				debugoverlay->AddTextOverlayRGB(m_vecTextOrigin, 0, 0, 255, 255, 255, 255, m_szMessageText); // Draw text on screen
			}
		}
	}
}