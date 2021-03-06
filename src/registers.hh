#ifndef DEVANIX_REG
#define DEVANIX_REG

#include <string>

// Registros que seran usados por el generador de codigo
enum Reg{
  zero,
  
  a0,
  a1,
  a2,
  a3,

  v0,
  v1,  

  t0,
  t1,
  t2,
  t3,
  t4,
  t5,
  t6,
  t7,

  s0,
  s1,
  s2,
  s3,
  s4,
  s5,
  s6,
  s7,

  t8,
  t9,

  gp, // global pointer 
  sp, // stack pointer
  fp, // frame pointer
  ra,  // return address 

  f0,
  f1,
  f2,
  f3,
  f4,
  f5,
  f6,
  f7,
  f8,
  f9,
  f10,
  f11,
  f12,
  f13,
  f14,
  f15,
  f16,
  f17,
  f18,
  f19,
  f20,
  f21,
  f22,
  f23,
  f24,
  f25,
  f26,
  f27,
  f28,
  f29,
  f30,
  f31

};

std::string regToString(Reg r);

bool isFloatReg(Reg r);

#endif
