

#define CTRL_BTN_W		(40)
#define CTRL_BTN_BC_W	(100)
#define CTRL_TEXT_W		(100)
/************************************************************************/
/*     隧道布局                                                          */
/************************************************************************/

#define LAYOUT_TOP_TUNNEL (35)	



/************************************************************************/
/*     设备布局                                                         */
/************************************************************************/
#define DEVICE_2_TUNNEL_OFFSET	(85)
#define LAYOUT_TOP_DEVICE	(LAYOUT_TOP_TUNNEL + DEVICE_2_TUNNEL_OFFSET)

#if 0

#define TOP_BC_UP		(135)
#define TOP_ERROR_UP	(TOP_BC_UP + 23)
#define TOP_CALL_UP		(TOP_BC_UP + 40)
#define TOP_NAME_UP		(TOP_BC_UP + 85)
#define TOP_PERSON_UP	(TOP_BC_UP + 105)


#define TOP_BC_DOWN		(415/*TOP_BC_UP + 250*/)
#define TOP_PERSON_DOWN	(TOP_BC_DOWN - 25)
#define TOP_ERROR_DOWN	(TOP_BC_DOWN + 23)
#define TOP_CALL_DOWN	(TOP_BC_DOWN + 40)
#define TOP_NAME_DOWN	(TOP_BC_DOWN + 85)

#else
#define TOP_BC_UP		(LAYOUT_TOP_DEVICE + 15)
#define TOP_ERROR_UP	(TOP_BC_UP + 23)
#define TOP_KM_UP		(TOP_BC_UP + 40)
#define TOP_CALL_UP		(TOP_BC_UP + 65)
#define TOP_NAME_UP		(TOP_BC_UP + 105)
#define TOP_PERSON_UP	(TOP_BC_UP + 125)

#if _DEBUG 
#define TOP_BC_DOWN		(350)
#else 
//#define TOP_BC_DOWN		(500)
#define TOP_BC_DOWN		(500)
#endif
#define TOP_ERROR_DOWN	(TOP_BC_DOWN + 23)
#define TOP_KM_DOWN		(TOP_BC_DOWN + 40)
#define TOP_CALL_DOWN	(TOP_BC_DOWN + 65)
#define TOP_NAME_DOWN	(TOP_BC_DOWN + 105)
#define TOP_PERSON_DOWN	(TOP_BC_DOWN - 25)
#define TOP_BUTTON_BOTTOM	(TOP_BC_DOWN + 130)
#define TOP_DEVICE_BOTTOM	(TOP_BC_DOWN + 180)
#endif

#define TOP_ARROW_UP	(TOP_PERSON_UP + (TOP_PERSON_DOWN - TOP_PERSON_UP) / 2 - 35)

#if 0
// 服务区位置
#define TOP_BC_UP_BC		(180)
#define TOP_ERROR_UP_BC		(TOP_BC_UP_BC + 90)
#define TOP_CALL_UP_BC		(TOP_BC_UP_BC + 40)
#define TOP_NAME_UP_BC		(TOP_BC_UP_BC + 110)

#define TOP_BC_DOWN_BC		(TOP_BC_UP_BC + 240)
#define TOP_ERROR_DOWN_BC	(TOP_BC_DOWN_BC + 90)
#define TOP_CALL_DOWN_BC	(TOP_BC_DOWN_BC + 40)
#define TOP_NAME_DOWN_BC	(TOP_BC_DOWN_BC + 110)
//#define TOP_ARROW_UP_BC		(318-5)
#define TOP_ARROW_UP_BC		(TOP_NAME_UP_BC + (TOP_BC_DOWN_BC - TOP_NAME_UP_BC) / 2 - 10)
#define TOP_BTN_OFFSET_BC	(450)


#else

// 服务区位置
#define TOP_BC_UP_BC		(LAYOUT_TOP_DEVICE + 30)
#define TOP_ERROR_UP_BC		(TOP_BC_UP_BC + 90)
#define TOP_KM_UP_BC		(TOP_BC_UP_BC + 40)
#define TOP_CALL_UP_BC		(TOP_BC_UP_BC + 65)
#define TOP_NAME_UP_BC		(TOP_BC_UP_BC + 110)
#define TOP_PERSON_UP_BC	(TOP_BC_UP_BC + 125)

#if _DEBUG 
#define TOP_BC_DOWN_BC		(350)
#else 
//#define TOP_BC_DOWN		(500)
#define TOP_BC_DOWN_BC		(500)
#endif


#define TOP_ERROR_DOWN_BC		(TOP_BC_DOWN_BC + 90)
#define TOP_KM_DOWN_BC			(TOP_BC_DOWN_BC + 40)
#define TOP_CALL_DOWN_BC		(TOP_BC_DOWN_BC + 65)
#define TOP_NAME_DOWN_BC		(TOP_BC_DOWN_BC + 110)
#define TOP_PERSON_DOWN_BC		(TOP_BC_DOWN_BC - 25)
#define TOP_BUTTON_BOTTOM_BC	(TOP_BC_DOWN_BC + 130)
#define TOP_DEVICE_BOTTOM_BC	(TOP_BC_DOWN_BC + 180)


#define TOP_ARROW_UP_BC			(TOP_PERSON_UP_BC + (TOP_PERSON_DOWN_BC - TOP_PERSON_UP_BC) / 2 - 35)


#endif




/************************************************************************/
/*     电话布局                                                          */
/************************************************************************/
#define TOP_CALL_LAYOUT (TOP_DEVICE_BOTTOM + 10)