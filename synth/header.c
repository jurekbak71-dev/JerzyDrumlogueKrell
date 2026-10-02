#include "unit.h"
#define P(lo,hi,init,type,name) {lo,hi,0,init,type,0,0,0,{name}}
const __unit_header unit_header_t unit_header={
.header_size=sizeof(unit_header_t),.target=UNIT_TARGET_PLATFORM|k_unit_module_synth,
.api=UNIT_API_VERSION,.dev_id=0x4A424B31U,.unit_id=0x4B524C31U,.version=0x10000U,
.name="JerzyKrell",.num_presets=0,.num_params=24,.params={
P(24,84,48,k_unit_param_type_midi_note,"NOTE"),
P(0,5,4,k_unit_param_type_strings,"CHORD"),
P(0,1,0,k_unit_param_type_strings,"MODE"),
P(0,100,70,k_unit_param_type_percent,"VELOCITY"),
P(10,8000,1400,k_unit_param_type_msec,"ATTACK"),
P(0,16000,3000,k_unit_param_type_msec,"HOLD"),
P(50,16000,6500,k_unit_param_type_msec,"RELEASE"),
P(0,100,25,k_unit_param_type_percent,"VARIATION"),
P(0,100,25,k_unit_param_type_percent,"FM"),
P(0,7,3,k_unit_param_type_strings,"RATIO"),
P(0,100,25,k_unit_param_type_percent,"FOLD"),
P(100,12000,2800,k_unit_param_type_hertz,"TONE"),
P(500,20000,4800,k_unit_param_type_msec,"MOTION"),
P(0,100,55,k_unit_param_type_percent,"DEPTH"),
P(0,30,8,k_unit_param_type_cents,"DRIFT"),
P(0,100,70,k_unit_param_type_percent,"LPG"),
P(0,100,85,k_unit_param_type_percent,"SPREAD"),
P(0,30,7,k_unit_param_type_cents,"DETUNE"),
P(0,75,35,k_unit_param_type_percent,"ECHO"),
P(0,85,50,k_unit_param_type_percent,"FEEDBACK"),
P(0,32767,71,k_unit_param_type_none,"SEED"),
P(0,100,12,k_unit_param_type_percent,"AIR"),
P(0,100,80,k_unit_param_type_percent,"LEVEL"),
P(0,1,0,k_unit_param_type_onoff,"FREEZE")
}};
