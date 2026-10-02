#include "synth.h"
#include <cassert>
#include <cstdio>
#include <memory>
#include <vector>
#include <algorithm>
static std::unique_ptr<Synth> make(){
 auto s=std::unique_ptr<Synth>(new Synth);unit_runtime_desc_t d{};d.samplerate=48000;d.output_channels=2;assert(s->Init(&d)==0);return s;
}
static std::vector<float> render(Synth& s,unsigned n,unsigned chunk=64){
 std::vector<float> x(n*2);for(unsigned i=0;i<n;){unsigned b=std::min(chunk,n-i);s.Render(x.data()+2*i,b);i+=b;}
 for(float v:x)assert(std::isfinite(v)&&std::fabs(v)<1);
 return x;
}
static float energy(const std::vector<float>& x){float sum=0;for(float v:x)sum+=v*v;return sum/x.size();}
static void plain(Synth& s,int mode=0){
 s.setParameter(Synth::Chord,0);s.setParameter(Synth::Mode,mode);s.setParameter(Synth::Attack,10);s.setParameter(Synth::Hold,100);s.setParameter(Synth::Release,50);
 for(auto i:{Synth::Variation,Synth::FM,Synth::Fold,Synth::Depth,Synth::Drift,Synth::LPG,Synth::Detune,Synth::Echo,Synth::Feedback,Synth::Air,Synth::Spread})s.setParameter(i,0);
 s.Reset();render(s,64);
}
int main(){
 auto s=make();assert(energy(render(*s,48000))==0);
 plain(*s);s->GateOn(127);assert(energy(render(*s,2400))>.0001f);
 s->GateOff();assert(energy(render(*s,2400))>.0001f);render(*s,20000);assert(energy(render(*s,1000))==0);
 plain(*s,1);s->GateOn(127);render(*s,12000);assert(energy(render(*s,2400))>.0001f);s->GateOff();render(*s,4000);assert(energy(render(*s,1000))==0);
 // MIDI voice remains held when sequencer gate closes.
 s->NoteOn(60,127);render(*s,2400);s->GateOff();assert(energy(render(*s,2400))>.0001f);s->NoteOff(61);assert(energy(render(*s,2400))>.0001f);s->NoteOff(60);render(*s,4000);assert(energy(render(*s,1000))==0);
 // NOTE is captured at the trigger, before the next Motion value.
 plain(*s,1);s->setParameter(Synth::Note,48);s->GateOn(127);s->setParameter(Synth::Note,60);auto low=render(*s,48000);
 s->AllNoteOff();render(*s,64);s->GateOn(127);auto high=render(*s,48000);
 auto crossings=[](const std::vector<float>& x){unsigned n=0;for(unsigned i=10000;i+2<x.size();i+=2)if(x[i]<=0&&x[i+2]>0)++n;return n;};
 assert(crossings(high)>1.95f*crossings(low)&&crossings(high)<2.05f*crossings(low));
 // Panic also removes delay memory; queue overflow recovers to silence.
 s->setParameter(Synth::Echo,75);render(*s,48000);s->AllNoteOff();assert(energy(render(*s,48000))==0);
 for(int i=0;i<100;++i){s->GateOn(127);}
 assert(energy(render(*s,1000))==0);
 // Same event schedule is independent of host buffer geometry.
 auto a=make(),b=make();a->GateOn(100);b->GateOn(100);assert(render(*a,48000,64)==render(*b,48000,127));
 // Full parameter limits, chord stealing, pitch bend and tempo.
 for(int extreme:{-99999,99999}){s=make();for(unsigned i=0;i<Synth::Count;++i)s->setParameter(i,extreme);s->SetTempo(20u*65536u);s->PitchBend(16383);for(int n=0;n<20;++n){s->GateOn(127);render(*s,4096);}s->AllNoteOff();assert(energy(render(*s,48000))==0);}
 unit_runtime_desc_t d{};d.samplerate=44100;d.output_channels=2;assert(s->Init(&d)==k_unit_err_samplerate);d.samplerate=48000;d.output_channels=1;assert(s->Init(&d)==k_unit_err_geometry);
 puts("PASS: silence, sequencer/MIDI gates, Bloom/Gate envelopes, Motion pitch capture, panic/overflow, buffer invariance and parameter extremes");
}
