#ifndef __EFFECTS_H_
#define __EFFECTS_H_

// The effect order here must match the HTML for the web server
typedef enum Effect {
    INVALID = 0,
    OFF,
    CHARGING,
    WAIT_FOR_CHARGE,
    NOISE,
    EFFECT_COUNT_OVERFLOW // Used to check invalid requests
} Effect;

#define TASK_BASE_UPDATE_INTERVAL   20 // ms
#define DEFAULT_UPDATE_INTERVAL     5 // In units of TASK_BASE_UPDATE_INTERVAL

// Define effect parameters with preprocessor for now, might be worth implementing with object
// OFF 
#define OFF_UPDATE_INTERVAL   DEFAULT_UPDATE_INTERVAL

// CHARGING
#define CHARGING_UPDATE_INTERVAL    DEFAULT_UPDATE_INTERVAL

// WAIT_FOR_CHARGE
#define WAIT_FOR_CHARGE_UPDATE_INTERVAL DEFAULT_UPDATE_INTERVAL

// NOISE
#define NOISE_UPDATE_INTERVAL   10 // 200 ms


#endif // __EFFECTS_H_