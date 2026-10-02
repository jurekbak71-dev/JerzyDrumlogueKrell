#pragma once
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include "unit.h"
// Serialized control callbacks produce events; Render owns all DSP state.
class Synth {
 public:
 enum {Note,Chord,Mode,Velocity,Attack,Hold,Release,Variation,FM,Ratio,Fold,Tone,Motion,Depth,Drift,LPG,Spread,Detune,Echo,Feedback,Seed,Air,Level,Freeze,Count};
 Synth(){for(unsigned i=0;i<Count;++i)p_[i].store(def()[i]);}
 int8_t Init(const unit_runtime_desc_t* d){if(!d)return k_unit_err_undef;if(d->samplerate!=48000)return k_unit_err_samplerate;if(d->output_channels!=2)return k_unit_err_geometry;for(unsigned i=0;i<Table;++i)table_[i]=std::sin(6.28318530718f*i/Table);clear();return k_unit_err_none;}
 void Reset(){panic_.store(true);} void Teardown(){Reset();} void Suspend(){Reset();} void Resume(){}
 void setParameter(uint8_t i,int32_t v){if(i<Count)p_[i].store(bound(v,lo()[i],hi()[i]));}
 int32_t getParameterValue(uint8_t i)const{return i<Count?p_[i].load():0;}
 const char* getParameterStrValue(uint8_t i,int32_t v)const{static const char* chords[]={"Single","Fifth","Minor","Major","Sus2","Quartal"};static const char* modes[]={"Bloom","Gate"};static const char* ratios[]={"0.5","1","1.5","2","3","4","5","7"};if(i==Chord)return chords[bound(v,0,5)];if(i==Mode)return modes[bound(v,0,1)];if(i==Ratio)return ratios[bound(v,0,7)];return nullptr;}
 const uint8_t* getParameterBmpValue(uint8_t,int32_t)const{return nullptr;}
 void SetTempo(uint32_t t){if(t>=20u*65536u&&t<=400u*65536u)tempo_.store(t);}
 void GateOn(uint8_t v){if(v)push(0,uint8_t(p_[Note].load()),v,true);}
 void GateOff(){push(1,0,0,true);}
 void NoteOn(uint8_t n,uint8_t v){if(n==255){GateOn(v);return;}if(v)push(0,n,v,false);else NoteOff(n);}
 void NoteOff(uint8_t n){push(1,n,0,false);}
 void AllNoteOff(){panic_.store(true);}
 void PitchBend(uint16_t b){bend_.store(b>16383?16383:b);}
 void ChannelPressure(uint8_t){} void Aftertouch(uint8_t,uint8_t){}
 void LoadPreset(uint8_t){} uint8_t getPresetIndex()const{return 0;} static const char* getPresetName(uint8_t){return nullptr;}
 void Render(float* out,size_t frames){
  if(!out)return;
  int32_t q[Count];for(unsigned i=0;i<Count;++i)q[i]=p_[i].load();
  if(panic_.exchange(false)){clear();read_.store(write_.load(std::memory_order_acquire),std::memory_order_release);}
  if(seed_!=q[Seed]){seed_=q[Seed];rng_=uint32_t(seed_)+0x9e3779b9u;}
  unsigned r=read_.load(std::memory_order_relaxed),w=write_.load(std::memory_order_acquire);
  while(r!=w){const Event e=events_[r];r=(r+1)%Queue;if(e.type==0)trigger(q,e);else for(auto& v:voices_)if(v.stage&&v.gated&&v.seq==e.seq&&(e.seq||v.note==e.note))release(v);}
  read_.store(r,std::memory_order_release);
  static const float ratios[]={.5f,1,1.5f,2,3,4,5,7};
  const float detune=std::pow(2.f,q[Detune]/1200.f),bend=std::pow(2.f,(int(bend_.load())-8192)/49152.f);
  const unsigned delayFrames=unsigned(fmin(47999.0,48000.0*60.0/(tempo_.load()/65536.0)*.75));
  for(size_t f=0;f<frames;++f){
   for(unsigned i=0;i<Count;++i)smooth_[i]+=(q[i]-smooth_[i])*.001f;
   float l=0,rout=0;
   for(auto& v:voices_){
    if(!v.stage)continue;
    if(v.stage==1){v.env+=v.attack;if(v.env>=1){v.env=1;v.stage=2;}}
    else if(v.stage==2){if(!v.gated){if(v.hold)v.hold--;else release(v);}}
    else{v.env-=v.release;if(v.env<=0){v.stage=0;continue;}}
    if(!q[Freeze]){v.progress+=v.speed;if(v.progress>=1){v.progress-=1;for(unsigned j=0;j<3;++j){v.from[j]=v.to[j];v.to[j]=randf();}v.speed=1.f/(48.f*q[Motion]*(1.f+.25f*randf()));}}
    const float t=v.progress*v.progress*(3-2*v.progress);float cv[3];for(unsigned j=0;j<3;++j)cv[j]=v.from[j]+(v.to[j]-v.from[j])*t;
    const float depth=smooth_[Depth]*.01f;
    const float inc=v.increment*bend*(1.f+cv[2]*smooth_[Drift]*.00057762f);
    const float fm=fmax(0.f,fmin(.6f,smooth_[FM]*.0035f+cv[0]*depth*.16f));
    float x=.5f*(sine(v.phase+sine(v.mod)*fm)+sine(v.other+sine(v.mod)*fm));
    x*=1.f+smooth_[Fold]*.025f+cv[1]*depth*.35f;
    if(x>1)x=2-x;else if(x< -1)x=-2-x;
    if(x>1)x=2-x;else if(x< -1)x=-2-x;
    x+=randf()*smooth_[Air]*.0004f;
    const float env=v.env*v.env*(3-2*v.env);
    const float cutoff=fmax(40.f,fmin(16000.f,smooth_[Tone]*(1+cv[1]*depth*.6f)*(1-smooth_[LPG]*.009f*(1-env))));
    const float a=cutoff/(cutoff+7639.437f);
    v.filter+=a*(x-v.filter);v.filter2+=a*(v.filter-v.filter2);
    x=v.filter2*env*v.velocity*.17f;
    const float pan=fmax(0.f,fmin(1.f,.5f+(v.pan+cv[2]*.2f)*smooth_[Spread]*.01f));
    float vl=x*(1-pan),vr=x*pan;
    if(v.tail){vl+=v.tailL*v.tail/128.f;vr+=v.tailR*v.tail/128.f;--v.tail;}
    v.lastL=vl;v.lastR=vr;l+=vl;rout+=vr;
    v.phase=wrap(v.phase+inc);v.other=wrap(v.other+inc*detune);v.mod=wrap(v.mod+inc*ratios[q[Ratio]]);
   }
   const unsigned rd=(delayWrite_+Delay-delayFrames)%Delay;
   const float dl=filled_>=delayFrames?delay_[0][rd]:0,dr=filled_>=delayFrames?delay_[1][rd]:0;
   delay_[0][delayWrite_]=l+dr*smooth_[Feedback]*.01f;delay_[1][delayWrite_]=rout+dl*smooth_[Feedback]*.01f;
   delayWrite_=(delayWrite_+1)%Delay;if(filled_<Delay)++filled_;
   const float a=(l+dl*smooth_[Echo]*.01f)*smooth_[Level]*.01f,b=(rout+dr*smooth_[Echo]*.01f)*smooth_[Level]*.01f;
   out[2*f]=a/(1+std::fabs(a));out[2*f+1]=b/(1+std::fabs(b));
  }
 }
 private:
 static constexpr unsigned Table=2048,Delay=48000,Queue=64;
 struct Voice{unsigned stage=0,hold=0,tail=0;uint8_t note=0;bool seq=false,gated=false;float env=0,attack=0,release=0,releaseTime=0,increment=0,phase=0,other=0,mod=0,velocity=0,pan=0,filter=0,filter2=0,progress=0,speed=0,from[3]{},to[3]{},lastL=0,lastR=0,tailL=0,tailR=0;};
 struct Event{uint8_t type,note,velocity;bool seq;};
 std::atomic<int32_t> p_[Count];std::atomic<unsigned> write_{0},read_{0};std::atomic<bool> panic_{false};std::atomic<uint32_t> tempo_{120u*65536u},bend_{8192};
 Event events_[Queue]{};Voice voices_[6]{};float table_[Table]{},delay_[2][Delay]{},smooth_[Count]{};unsigned delayWrite_=0,filled_=0,next_=0;uint32_t rng_=1;int seed_=-1;
 static int32_t bound(int32_t x,int32_t a,int32_t b){return x<a?a:(x>b?b:x);}
 static const int32_t* def(){static const int32_t a[]={48,4,0,70,1400,3000,6500,25,25,3,25,2800,4800,55,8,70,85,7,35,50,71,12,80,0};return a;}
 static const int32_t* lo(){static const int32_t a[]={24,0,0,0,10,0,50,0,0,0,0,100,500,0,0,0,0,0,0,0,0,0,0,0};return a;}
 static const int32_t* hi(){static const int32_t a[]={84,5,1,100,8000,16000,16000,100,100,7,100,12000,20000,100,30,100,100,30,75,85,32767,100,100,1};return a;}
 void clear(){for(auto& v:voices_)v=Voice{};delayWrite_=filled_=next_=0;seed_=-1;bend_.store(8192);for(unsigned i=0;i<Count;++i)smooth_[i]=float(p_[i].load());}
 void push(uint8_t type,uint8_t n,uint8_t velocity,bool seq){unsigned w=write_.load(std::memory_order_relaxed),next=(w+1)%Queue;if(next==read_.load(std::memory_order_acquire)){panic_.store(true);return;}events_[w]={type,n,uint8_t(velocity>127?127:velocity),seq};write_.store(next,std::memory_order_release);}
 uint32_t random(){rng_^=rng_<<13;rng_^=rng_>>17;rng_^=rng_<<5;return rng_;}
 float randf(){return float(random()&65535)*(2.f/65535.f)-1.f;}
 static float wrap(float x){while(x>=1)x-=1;while(x<0)x+=1;return x;}
 float sine(float p)const{float x=wrap(p)*Table;unsigned i=unsigned(x);return table_[i]+(table_[(i+1)%Table]-table_[i])*(x-i);}
 static void release(Voice& v){v.stage=3;v.gated=false;v.release=v.env/(48.f*v.releaseTime);}
 void trigger(const int32_t* q,const Event& e){
  static const int intervals[6][3]={{0,0,0},{0,7,12},{0,3,7},{0,4,7},{0,2,7},{0,5,10}};
  const unsigned count=q[Chord]?3:1;
  for(unsigned j=0;j<count;++j){unsigned index=next_;for(unsigned k=0;k<6;++k)if(!voices_[(next_+k)%6].stage){index=(next_+k)%6;break;}next_=(index+1)%6;
   Voice& v=voices_[index];float tl=v.lastL,tr=v.lastR;bool stolen=v.stage!=0;v=Voice{};if(stolen){v.tail=128;v.tailL=tl;v.tailR=tr;}
   v.stage=1;v.seq=e.seq;v.note=e.note;v.gated=q[Mode];
   v.increment=440.f*std::pow(2.f,(bound(e.note+intervals[q[Chord]][j],0,96)-69)/12.f)/48000.f;
   const float variation=q[Variation]*.005f;
   v.attack=1.f/(48.f*q[Attack]*(1+variation*randf()));v.hold=unsigned(48.f*q[Hold]*(1+variation*randf()));v.releaseTime=q[Release]*(1+variation*randf());
   v.velocity=1-q[Velocity]*.01f*(1-e.velocity/127.f);v.pan=randf()*.45f;v.speed=1.f/(48.f*q[Motion]);
   for(unsigned k=0;k<3;++k){v.from[k]=randf();v.to[k]=randf();}
  }
 }
};
