// Random.c
// Runs on TM4C1294
// Linear congruential generator from Numerical Recipes
// by Press et al.  To use this module, call Random_Init()
// once with a seed and (Random()>>24)%60 over and over to
// get a new random number from 0 to 59.
// Jonathan Valvano
// April 23, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
   Program 2.8, Section 2.8

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
uint32_t M;

//------------Random_Init------------
// Initialize the random number generator with the given seed.
// Input: seed  new seed value for random number generation
// Output: none
void Random_Init(uint32_t seed){
  M = seed;
}

//------------Random------------
// Generate a semi-random number.  The lower bits might not be
// random, so right shift by some amount before scaling to the
// desired range.
// Input: none
// Output: 32-bit semi-random number
uint32_t Random(void){
  M = 1664525*M+1013904223;
  return(M);
}
