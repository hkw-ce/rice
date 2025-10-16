#ifndef _APP_H_
#define _APP_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "platform.h"

#define BTN1_SINGLE_CLICK_FLAG1 (1 << 1)
#define BTN2_SINGLE_CLICK_FLAG2 (1 << 2)
#define EVENT_ENTER_MENU_FLAG (1 << 3)
#define BTN1_MODE_START_FLAG4 (1 << 4)
#define EVENT_STOP_FLAG (1 << 5)
#define EVENT_START_FLAG (1 << 6)
#define EVENT_STOP_NOW_FLAG (1 << 7)
#define EVENT_COUNT_DOWN_FLAG (1 << 8)
#define EVENT_COUNTDOWN_UPDATE0 (1 << 9)
#define EVENT_MODE_RUN0 (1 << 10)
  //// 事件分发器 event标志位
  // #define EVENT_OVER_TEMP (1 << 0)     // 过温保护
  // #define EVENT_OVER_CURRENT (1 << 1)  // 过流保护
  // #define EVENT_OVER_VOLTAGE (1 << 2)  // 过压保护
  // #define EVENT_PAN_REMOVED (1 << 3)   // 锅具移除
  // #define EVENT_COOK_PAUSE (1 << 4)    // 烹饪暂停
  // #define EVENT_COOK_RESUME (1 << 5)   // 烹饪继续
  // #define EVENT_COOK_DONE (1 << 6)     // 烹饪完成
  // #define EVENT_DOOR_OPENED (1 << 7)   // 门盖打开
  // #define EVENT_DOOR_CLOSED (1 << 8)   // 门盖关闭
  // #define EVENT_FAN_ERROR (1 << 9)     // 风扇异常
  // #define EVENT_HEATER_ERROR (1 << 10) // 加热管异常
  // #define EVENT_SYSTEM_RESET (1 << 11) // 系统复位或重启
// 时间数值 → WS2812 灯号
	typedef struct
	{
			uint8_t key;     // 对应的按键
			uint8_t time_min; // 分钟数
			uint8_t led_idx;  // 对应的 WS2812 灯号
	} time_led_map_t;
	// 温度数值 → WS2812 灯号
	typedef struct
	{
			uint8_t key;     // 对应的按键
			uint16_t temp_c; // 摄氏度
			uint8_t led_idx; // 对应的 WS2812 灯号
	} temp_led_map_t;


  extern void app(void);
  // 菜单结构体
  typedef struct menu_item
  {
    char name[20];
    void (*action)(void);
    uint8_t index;
    struct menu_item *next;
    struct menu_item *parent;
    struct menu_item *child;
  } menu_item;

  struct base_dev_t
  {
    uint16_t run_time;
    uint16_t software_version;
    uint16_t hardware_version;
    uint16_t uuid;
    uint8_t error_code;
  };

  struct device_config_t
  {
    // 模式设置
    uint8_t mode_id;         // 当前模式编号
    uint16_t time_min;       // 当前模式时长（分钟）
    uint16_t count_time_min; // 当前模式预约时长（分钟）
    uint16_t temp_c;         // 当前模式温度（℃）
    uint8_t time_step;       // 时长步长（分钟）
    uint8_t temp_step;       // 温度步长（℃）

    // 状态标志
    bool is_running;   // 是否正在运行
    bool is_countdown; // 是否正在运行
    bool def_mode;     // 是否一键启动默认模式
    bool long_cancel;  // 是否长按取消
    bool allow_edit;   // 是否允许中途修改参数
    bool is_close;

    // 显示与提示
    bool beep;      // 是否有提示音
    bool show_mode; // 是否显示当前模式
    bool show_left; // 是否显示剩余时长
    bool show_temp; // 是否显示实时温度

    // 电池信息
    bool bat_ok;   // 是否检测到电池连接
    float bat_v;   // 电池电压
    float bat_i;   // 电池电流
    float bat_soc; // 电池电量百分比
    float heater_i;

    // 电池事件
    bool bat_add; // 增加电池
    bool bat_rm;  // 移除电池

    // 新增字段
    uint16_t remain_sec;  // 剩余时间，单位秒
    uint8_t fault_flag;   // 故障码，0正常，非0故障
    bool temp_sensor_err; // 温度传感器异常
    bool batt_sensor_err; // 电池传感器异常

    float temperature;
  };
  typedef struct
  {
    struct base_dev_t base;
    struct device_config_t dev_conf;
    /* 按键 事件控制块 */
//    struct rt_event btn_event;
  } env_t;
  extern env_t env;

#ifdef __cplusplus
}
#endif

#endif
