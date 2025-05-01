//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//


#include "cbase.h"
#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui/ISurface.h>
#include <vgui/IImage.h>
#include <vgui_controls/Label.h>

#include "tf_imagepanel.h"
#include "c_tf_player.h"

#include "tf_gamerules.h"
#include "sf/sf_vruksstupiduihack.h"

using namespace vgui;

DECLARE_BUILD_FACTORY( CTFImagePanel );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFImagePanel::CTFImagePanel( Panel *parent, const char *name ) : ScalableImagePanel( parent, name )
{
	for ( int i = 0; i < TF_TEAM_COUNT; i++ )
	{
		m_szTeamBG[i][0] = '\0';
	}
	m_szTeamCustomBG[0] = '\0';

	C_TFPlayer *pPlayer = ToTFPlayer( C_BasePlayer::GetLocalPlayer() );
	m_iBGTeam = pPlayer ? pPlayer->GetTeamNumber() : TEAM_UNASSIGNED;
	m_iBGTeamFromSettings = TEAM_INVALID;

	ListenForGameEvent( "localplayer_changeteam" );
	ListenForGameEvent( "colors_updated" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFImagePanel::ApplySettings( KeyValues *inResourceData )
{
	for ( int i = 0; i < TF_TEAM_COUNT; i++ )
	{
		Q_strncpy( m_szTeamBG[i], inResourceData->GetString( VarArgs("teambg_%d", i), "" ), sizeof( m_szTeamBG[i] ) );

		if ( m_szTeamBG[i] && m_szTeamBG[i][0] )
		{
			PrecacheMaterial( VarArgs( "vgui/%s", m_szTeamBG[i] ) );
		}
	}

	// @Kiwano adding support for custom (RGB) team colors in vgui elements
	Q_strncpy( m_szTeamCustomBG, inResourceData->GetString( "teambg_custom", "" ), sizeof( m_szTeamCustomBG ) );

	if ( m_szTeamCustomBG[0] )
	{
		PrecacheMaterial( VarArgs( "vgui/%s", m_szTeamCustomBG ) );
	}

	m_iBGTeamFromSettings = inResourceData->GetInt("teamoverride", TEAM_INVALID);

	BaseClass::ApplySettings( inResourceData );

	UpdateBGImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFImagePanel::UpdateBGImage( void )
{
	if(m_iBGTeamFromSettings != TEAM_INVALID)
	{
		m_iBGTeam = m_iBGTeamFromSettings;
	}
	
	if ( m_iBGTeam >= 0 && m_iBGTeam < TF_TEAM_COUNT )
	{
		if (TFGameRules())
		{
			if (m_iBGTeam == TF_TEAM_BLUE)
			{
				if (TFGameRules()->GetBlueTeamHasCustomColor() && m_szTeamCustomBG[0])
				{
					SetImage(m_szTeamCustomBG);
					return;
				}
			}
			else if (m_iBGTeam == TF_TEAM_RED)
			{
				if (TFGameRules()->GetRedTeamHasCustomColor() && m_szTeamCustomBG[0])
				{
					SetImage(m_szTeamCustomBG);
					return;
				}
			}
		}

		if ( m_szTeamBG[m_iBGTeam] && m_szTeamBG[m_iBGTeam][0] )
		{
			SetImage( m_szTeamBG[m_iBGTeam] );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFImagePanel::SetBGTeam( int iTeamNum )
{
	if (iTeamNum >= 0 && iTeamNum < TF_TEAM_COUNT)
	{
		m_iBGTeam = iTeamNum;
	}
}

void CTFImagePanel::PaintBackground()
{
	// @Kiwano This is a hack to get around usage of material proxies
	// in vgui elements not providing a pointer (can't set color)
	VruksStupidUIHack::SetTeamOverride(m_iBGTeam);
	BaseClass::PaintBackground();
	VruksStupidUIHack::SetTeamOverride(TEAM_INVALID);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFImagePanel::FireGameEvent( IGameEvent * event )
{
	if ( FStrEq( "localplayer_changeteam", event->GetName() ) )
	{
		C_TFPlayer *pPlayer = ToTFPlayer( C_BasePlayer::GetLocalPlayer() );
		m_iBGTeam = pPlayer ? pPlayer->GetTeamNumber() : TEAM_UNASSIGNED;
		UpdateBGImage();
	}
	else if (FStrEq( "colors_updated", event->GetName() ) )
	{
		UpdateBGImage();
	}
}
