class VPPESPTracker: ScriptedWidgetEventHandler 
{
    protected Widget           m_RootWidget;
    protected GridSpacerWidget m_SpacerGrid;
    protected CheckBoxWidget   m_CheckBox;
    protected TextWidget       m_ItemNameWidget;
    protected TextWidget       m_ItemDistanceWidget;
	protected SliderWidget     m_HealthInput;
	protected SliderWidget     m_BloodInput;
	protected TextWidget       m_GUIDInput;
	protected TextWidget       m_GUID64Input;
    protected string           m_ItemName;
    protected vector	       m_MarkerPositon;
    protected bool             m_IsMarkerVisible;
	private ref PlayerStatsData m_playerInfo;
	private PlayerBase player;
	Object 					   m_TrackerEntity;

    void VPPESPTracker(string itemName, Object trackedEntity, bool detailed, int color = -1, bool visible = true ) 
	{
        m_RootWidget = GetGame().GetWorkspace().CreateWidgets( "VPPAdminTools/GUI/Layouts/EspToolsUI/EspTracker.layout", null);
		m_RootWidget.SetHandler(this);
        m_SpacerGrid       = GridSpacerWidget.Cast( m_RootWidget.FindAnyWidget( "SpacerGrid" ) );

        m_CheckBox         = CheckBoxWidget.Cast( m_RootWidget.FindAnyWidget( "CheckBox" ) );
        m_ItemNameWidget   = TextWidget.Cast( m_SpacerGrid.FindAnyWidget( "ItemName" ) );
        m_ItemDistanceWidget = TextWidget.Cast( m_SpacerGrid.FindAnyWidget( "ItemDistance" ) );

        m_TrackerEntity    = trackedEntity;
		
		if(m_TrackerEntity.IsInherited(PlayerBase))
		{
			player = PlayerBase.Cast(m_TrackerEntity);
		}
        m_ItemName  	   = itemName;
        m_IsMarkerVisible  = visible;

        m_ItemNameWidget.SetText( m_ItemName );
		m_ItemNameWidget.SetColor( color );
        m_SpacerGrid.Update();
        m_ItemNameWidget.Update();
        m_ItemDistanceWidget.Update();
		
		m_RootWidget.SetSort(1023,true);
		GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert(this.DoUpdate);
    }

    void ~VPPESPTracker() 
	{
		GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove(this.DoUpdate);
        if (m_RootWidget != null) 
			m_RootWidget.Unlink();
    }
	
	void InitPlayerEspWidget(PlayerStatsData stats)
	{
		if (m_RootWidget != null)
			m_RootWidget.Unlink();

		m_RootWidget = GetGame().GetWorkspace().CreateWidgets( "VPPAdminTools/GUI/Layouts/EspToolsUI/EspTrackerDetailed.layout", null);
		m_RootWidget.SetHandler(this);
		m_SpacerGrid  = GridSpacerWidget.Cast( m_RootWidget );
		
		m_ItemNameWidget   	 = TextWidget.Cast( m_RootWidget.FindAnyWidget( "NameInput" ) );
		m_ItemDistanceWidget = TextWidget.Cast( m_RootWidget.FindAnyWidget( "DistanceInput" ) );
		m_HealthInput		 = SliderWidget.Cast( m_RootWidget.FindAnyWidget( "HealthInput" ) );
		m_BloodInput		 = SliderWidget.Cast( m_RootWidget.FindAnyWidget( "BloodInput" ) );
		m_GUID64Input		 = TextWidget.Cast( m_RootWidget.FindAnyWidget( "GUID64Input" ) );
			
		m_playerInfo = stats;
		m_ItemNameWidget.SetText( m_playerInfo.GetStat("Name") );
		m_HealthInput.SetCurrent(m_playerInfo.GetStat("Health").ToFloat());
		m_BloodInput.SetCurrent(m_playerInfo.GetStat("Blood").ToFloat());
		m_GUID64Input.SetText(m_playerInfo.GetStat("Steam64"));
		m_RootWidget.Update();
		m_RootWidget.SetSort(1023,true);
	}
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w != null && w == m_CheckBox)
		{
			autoptr EspToolsMenu espManager = EspToolsMenu.Cast(VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud)).GetSubMenuByType(EspToolsMenu));
			if (m_CheckBox.IsChecked())
			{
				//Color update
				m_CheckBox.SetColor(ARGB(150,0,255,21));
				//Add
				if (espManager)
					espManager.AddEntry(m_ItemName, m_TrackerEntity);
			}else{
				//Color update
				m_CheckBox.SetColor(ARGB(150,255,0,0));
				//Remove
				if (espManager)
					espManager.RemoveEntryByObject(m_TrackerEntity);
			}
			
			return true;
		}
		return false;
	}
	
    float CalcDistance() 
	{
		vector startPos    = m_TrackerEntity.GetPosition();
        vector SnappedPos  = g_Game.SnapToGround( Vector(startPos[0], startPos[1], startPos[2]) );
        vector fn    	   = Vector(SnappedPos[0], SnappedPos[1], SnappedPos[2]);
		
        return vector.Distance( GetGame().GetPlayer().GetPosition(), startPos );
    }

    void DoUpdate(float tDelta)
	{
		if (m_TrackerEntity == null)
		{
			delete this;
			return;
		}
		
        if (IsMarkerVisible() && m_RootWidget != NULL && GetGame().GetPlayer()) 
		{
			vector startPos    = m_TrackerEntity.GetPosition();
	        vector SnappedPos  = g_Game.SnapToGround( Vector(startPos[0], startPos[1], startPos[2]) );
	        vector fn    	   = Vector(SnappedPos[0], SnappedPos[1] , SnappedPos[2]);
			
            vector ScreenPosRelative = GetGame().GetScreenPosRelative(startPos);
            if( ScreenPosRelative[0] >= 1 || ScreenPosRelative[0] == 0 || ScreenPosRelative[1] >= 1 || ScreenPosRelative[1] == 0 ) {
                m_RootWidget.Show( false );
                return;
            } else if( ScreenPosRelative[2] < 0 ) {
                m_RootWidget.Show( false );
                return;
            } else {
                m_RootWidget.Show( true );
            }

            float pos_x,pos_y;

            vector ScreenPos = GetGame().GetScreenPos(startPos);
            pos_x = ScreenPos[0];
            pos_y = ScreenPos[1];

            pos_x = Math.Ceil(pos_x);
            pos_y = Math.Ceil(pos_y);

            m_RootWidget.SetPos(pos_x,pos_y);
            m_ItemDistanceWidget.SetText( "[" + Math.Floor(CalcDistance()).ToString() + "m]" );

            m_SpacerGrid.Update();
            m_ItemNameWidget.Update();
            m_ItemDistanceWidget.Update();
			
			if(m_playerInfo != null)
			{
				if(player != null && !player.IsAlive())
				{
					m_HealthInput.SetCurrent(0);
					m_BloodInput.SetCurrent(0);
				}
			
				if(player != null && player.IsAlive())
				{
					m_HealthInput.SetCurrent(player.GetTransferValues().GetHealth() * 100);
					m_BloodInput.SetCurrent(player.GetTransferValues().GetBlood() * 5000);
				}
			}
        }
		else if (m_RootWidget != NULL) 
		{
            m_RootWidget.Show(false);
        }
    }

    bool IsMarkerVisible() 
	{
        return m_IsMarkerVisible;
    }
	
	void SetChecked(bool state)
	{
		if (m_CheckBox != null)
			m_CheckBox.SetChecked(state);
	}
	
	bool IsChecked()
	{
		if (m_CheckBox != null)
			return m_CheckBox.IsChecked();
		
		return false;
	}

    void SetMarkerVisible(bool visible) 
	{
        m_IsMarkerVisible = visible;
   }
	
	bool IsTrackedObject(Object obj)
	{
		return obj.GetNetworkIDString() == m_TrackerEntity.GetNetworkIDString();
	}
	
	Object GetTrackingObject()
	{
		return m_TrackerEntity;
	}
};