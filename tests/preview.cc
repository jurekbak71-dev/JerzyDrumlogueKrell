#include "synth.h"
#include <cstdio>
#include <memory>
#include <cstdint>
int main(){
 auto s=std::unique_ptr<Synth>(new Synth);unit_runtime_desc_t d{};d.samplerate=48000;d.output_channels=2;if(s->Init(&d))return 1;
 s->setParameter(Synth::Hold,1800);s->setParameter(Synth::Release,5000);s->SetTempo(90u*65536u);
 const unsigned triggers[]={0,192000,384000,576000,768000};
 const int notes[]={48,50,43,53,48};
 float out[128];unsigned next=0;
 for(unsigned f=0;f<24*48000;f+=64){
  if(next<5&&f==triggers[next]){s->setParameter(Synth::Note,notes[next]);s->GateOn(110);++next;}
  if(f%192000==6400)s->GateOff();
  s->Render(out,64);
  int16_t pcm[128];for(unsigned i=0;i<128;++i)pcm[i]=int16_t(out[i]*32767.f);
  if(std::fwrite(pcm,sizeof(pcm),1,stdout)!=1)return 2;
 }
}
