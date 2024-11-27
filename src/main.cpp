/*######################################################################
  //#  G0B1T: uC EXAMPLES. 2024.
  //######################################################################
  //# Copyright (C) 2024. F.E.Segura-Quijano (FES) fsegura@uniandes.edu.co
  //#
  //# This program is free software: you can redistribute it and/or modify
  //# it under the terms of the GNU General Public License as published by
  //# the Free Software Foundation, version 3 of the License.
  //#
  //# This program is distributed in the hope that it will be useful,
  //# but WITHOUT ANY WARRANTY; without even the implied warranty of
  //# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  //# GNU General Public License for more details.
  //#
  //# You should have received a copy of the GNU General Public License
  //# along with this program.  If not, see <http://www.gnu.org/licenses/>
  //####################################################################*/

//=======================================================
//  LIBRARY Definition
//=======================================================
#include <Arduino.h>
#include <iostream>
#include <tuple>
using namespace std;

#define REALMATRIX // Variable to use real matrix. Comment to not use it.

#ifdef REALMATRIX
#include "LedControl.h"
/* Pin definition for MAX72XX.
 ARDUINO pin 12 is connected to the DataIn  - In ESP32 pin 23
 ARDUINO pin 11 is connected to the CLK     - In ESP32 pin 18
 ARDUINO pin 10 is connected to LOAD        - In ES32 pin 5
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(23,18,5,1);
#endif

enum State {
            STATE_RESET,
            STATE_START,
            STATE_CLEAR,
            STATE_CHECK,
            STATE_GO,
            STATE_LEFT,
            STATE_RIGHT,
            STATE_LOST,
            STATE_WIN,
            STATE_SHOW_LEVEL
            };

enum Key {
          KEY_RESET,
          KEY_START,
          KEY_LEFT,
          KEY_RIGHT,
          KEY_VOID
          };

enum Status {
            STATUS_LOST,
            STATUS_NEXT,
            STATUS_LEVEL,
            STATUS_WIN
            };

/* Registers to background cars.*/

byte RegBACKGTYPE_dataRANDOM;
byte RegBACKGTYPE_dataZEROS = B00000000;

unsigned long delaytime = 2000;

int i = 0;
int level = 0;
int points = 0;

State state = STATE_RESET;
Key key = KEY_RESET;
Status status = STATUS_NEXT;

int incomingByte;

byte RegMatrix[8];
byte *pointerRegMatrix;

byte RegCar[1];
byte *pointerRegCar;

byte ShiftDir[1];
byte *pointerShiftDir;

byte levelMatrix[8];
byte *pointerLevelMatrix;

void setup()
{
#ifdef REALMATRIX
  /* The MAX72XX is in power-saving mode on startup, we have to do a wakeup call. */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values. */
  lc.setIntensity(0, 8);
  /* Clear the display. */
  lc.clearDisplay(0);
#endif
  /* Serial port initialization. */
  Serial.begin(9600);

  /* Pointer to use Matrix between functions. */
  pointerRegMatrix = &RegMatrix[0];

  /* Pointer to use VectorCar between functions. */
  pointerRegCar = &RegCar[0];

  /* Pointer to use shift dir between functions */
  pointerShiftDir = &ShiftDir[0];

  pointerLevelMatrix = &levelMatrix[0];
}

void writeResetMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */

  /* Here is the data to reset matrix */
  pointerRegMatrix[7] = B11111111;
  pointerRegMatrix[6] = B11111111;
  pointerRegMatrix[5] = B11111111;
  pointerRegMatrix[4] = B11111111;
  pointerRegMatrix[3] = B11111111;
  pointerRegMatrix[2] = B11111111;
  pointerRegMatrix[1] = B11111111;
  pointerRegMatrix[0] = B11111111;
  /* Here is the data to reset bottomCar */
  pointerRegCar[0] = B00000000;
}

void writeStartMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Aquí están los datos para formar la 'S' sin usar los bordes */
  pointerRegMatrix[7] = B00000000; // Primera fila (vacía)
  pointerRegMatrix[6] = B00111100; // Segunda fila
  pointerRegMatrix[5] = B00000010; // Tercera fila
  pointerRegMatrix[4] = B00011110; // Cuarta fila
  pointerRegMatrix[3] = B00100000; // Quinta fila
  pointerRegMatrix[2] = B01000000; // Sexta fila
  pointerRegMatrix[1] = B00111100; // Séptima fila
  pointerRegMatrix[0] = B00000000; // Última fila (vacía)


  /* Datos para bottomCar (si es necesario para otra parte de tu sistema) */
  pointerRegCar[0] = B00000000;
}

void writeWinMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */

  /* Here is the data to start matrix */
  pointerRegMatrix[7] = B01111110;
  pointerRegMatrix[6] = B10000001;
  pointerRegMatrix[5] = B10011001;
  pointerRegMatrix[4] = B10000001;
  pointerRegMatrix[3] = B10100101;
  pointerRegMatrix[2] = B10011001;
  pointerRegMatrix[1] = B10000001;
  pointerRegMatrix[0] = B01111110;
/* Here is the data to start bottomCar */
  pointerRegCar[0] = B00000000;
}

void writeClearMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */

  /* Here is the data to clear matrix */
  pointerRegMatrix[7] = B00000000;
  pointerRegMatrix[6] = B00000000;
  pointerRegMatrix[5] = B00000000;
  pointerRegMatrix[4] = B00000000;
  pointerRegMatrix[3] = B00000000;
  pointerRegMatrix[2] = B00000000;
  pointerRegMatrix[1] = B00000000;
  pointerRegMatrix[0] = B00000000;
  /* Here is the data to clear bottomCar */
  pointerRegCar[0] = B00010000;
}

void writeLostMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */

  /* Here is the data to lost matrix */
  pointerRegMatrix[7] = B01111110;
  pointerRegMatrix[6] = B10000001;
  pointerRegMatrix[5] = B10011001;
  pointerRegMatrix[4] = B10000001;
  pointerRegMatrix[3] = B10011001;
  pointerRegMatrix[2] = B10100101;
  pointerRegMatrix[1] = B10000001;
  pointerRegMatrix[0] = B01111110;
  /* Here is the data to lost matrix */
  pointerRegCar[0] = B00000000;
}

void writeGoCarsMatrix(byte *pointerRegMatrix)
{
  /* Global variables. */
  int m;

  i = i + 1;
  /* Here is the data to start matrix */
  RegBACKGTYPE_dataRANDOM = random(1, 255);

  for (int m = 0; m < 7; m++)
  {
    pointerRegMatrix[m] = pointerRegMatrix[m + 1];
  }
  if (i % 2 == 0)
    pointerRegMatrix[7] = RegBACKGTYPE_dataRANDOM;
  else
    pointerRegMatrix[7] = RegBACKGTYPE_dataZEROS;
}

void writeCarBase(byte *pointerRegCar, byte *pointerShiftDir)
{
  /* Global variables. */
  int m;
  
  /* Here is the data to start matrix */
  if (pointerShiftDir[0] == B00000001)
  {
    if (pointerRegCar[0] == B00000001)
      pointerRegCar[0] = pointerRegCar[0];
    else
      pointerRegCar[0] = pointerRegCar[0] >> 1;
  }
  else if (pointerShiftDir[0] == B00000010)
  {
    if (pointerRegCar[0] == B10000000)
      pointerRegCar[0] = pointerRegCar[0];
    else
      pointerRegCar[0] = pointerRegCar[0] << 1;
  }
  else
    pointerRegCar[0] = pointerRegCar[0];
}

void writeLevelMatrix(byte *pointerLevelMatrix, int level) {
  
  switch(level)
  {
    case 0:
      pointerLevelMatrix[7] = B00000000;
      pointerLevelMatrix[6] = B01100000;
      pointerLevelMatrix[5] = B01100000;
      pointerLevelMatrix[4] = B00000000;
      pointerLevelMatrix[3] = B00000000;
      pointerLevelMatrix[2] = B00000000;
      pointerLevelMatrix[1] = B00000000;
      pointerLevelMatrix[0] = B00000000;
      break;

    case 1:
      pointerLevelMatrix[7] = B00000000;
      pointerLevelMatrix[6] = B01100110;
      pointerLevelMatrix[5] = B01100110;
      pointerLevelMatrix[4] = B00000000;
      pointerLevelMatrix[3] = B00000000;
      pointerLevelMatrix[2] = B00000000;
      pointerLevelMatrix[1] = B00000000;
      pointerLevelMatrix[0] = B00000000;
      break;
    
    case 2:
      pointerLevelMatrix[7] = B00000000;
      pointerLevelMatrix[6] = B01100110;
      pointerLevelMatrix[5] = B01100110;
      pointerLevelMatrix[4] = B00000000;
      pointerLevelMatrix[3] = B00000000;
      pointerLevelMatrix[2] = B01100000;
      pointerLevelMatrix[1] = B01100000;
      pointerLevelMatrix[0] = B00000000;
      break;
    
    case 3:
      pointerLevelMatrix[7] = B00000000;
      pointerLevelMatrix[6] = B01100110;
      pointerLevelMatrix[5] = B01100110;
      pointerLevelMatrix[4] = B00000000;
      pointerLevelMatrix[3] = B00000000;
      pointerLevelMatrix[2] = B01100110;
      pointerLevelMatrix[1] = B01100110;
      pointerLevelMatrix[0] = B00000000;
      break;
    
    case 1000:
      pointerLevelMatrix[7] = B00000000;
      pointerLevelMatrix[6] = B00000000;
      pointerLevelMatrix[5] = B00000000;
      pointerLevelMatrix[4] = B00000000;
      pointerLevelMatrix[3] = B00000000;
      pointerLevelMatrix[2] = B00000000;
      pointerLevelMatrix[1] = B00000000;
      pointerLevelMatrix[0] = B00000000;
      break;
    
  }
#ifdef REALMATRIX
  for (int m = 7; m >= 0; m--)
  {
    lc.setRow(0, m, pointerLevelMatrix[m]);
  }
#endif
}


void checkMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */
  byte check1, check2;

  check1 = pointerRegCar[0] ^ pointerRegMatrix[0];
  check2 = pointerRegCar[0] | pointerRegMatrix[0];

  
  if (pointerRegCar[0] == pointerRegMatrix[0])
    {status = STATUS_LOST;}
  else if (check1 != check2)
    {status = STATUS_LOST;}
  else if(points == 0){
      level = 0;
      delaytime = 4000;
      status = STATUS_LEVEL;
    }else if(points == 10){
      level=1;
      delaytime = 3000;
      status = STATUS_LEVEL;
    }else if(points == 25){
      level=2;
      delaytime = 1000;
      status = STATUS_LEVEL;
    }else if(points == 45){
      level=3;
      status = STATUS_WIN;
    }else{
      //points++;
      status = STATUS_NEXT;
    }
    points++;
    
}

void printBits(byte myByte)
{
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask  & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}

void PrintMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */
  int m;

  for (m = 7; m >= 1; m--)
  {
    printBits(pointerRegMatrix[m]);
    Serial.println();
  }
  printBits(pointerRegMatrix[0] | pointerRegCar[0]);
  Serial.println();
}

void PrintALLMatrix(byte *pointerRegMatrix, byte *pointerRegCar) {
  /* Global variables. */
  int m;

#ifdef REALMATRIX
  /* Display data one by one in matrix. */
  for (m = 7; m >= 1; m--)
  {
    lc.setRow(0, m, pointerRegMatrix[m]);
  }
  lc.setRow(0, m, (pointerRegMatrix[0] | pointerRegCar[0]));
#endif
  /* Display data one by one in console. */
  Serial.println("########");
  Serial.println("########");
  Serial.println("########");
  Serial.println("########");
  PrintMatrix(pointerRegMatrix, pointerRegCar);
  Serial.println("########");
}

void printLevelMatrix(int level) {
  // Validar que el nivel sea 1, 2 o 3
  if (level < 1 || level > 3) {
    Serial.println("Nivel no válido. Debe ser 1, 2 o 3.");
    return;
  }

  // Recorrer filas de la matriz
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      // Formar el número correspondiente con unos y ceros
      if (level == 1) {
        // Dibujar el "1" (columna central)
        if (col == 4 && row >= 2 && row <= 6) {
          Serial.print("1");
        } else {
          Serial.print("0");
        }
      } else if (level == 2) {
        // Dibujar el "2"
        if ((row == 2 && col >= 2 && col <= 5) || // Línea superior
            (row == 4 && col >= 2 && col <= 5) || // Línea central
            (row == 6 && col >= 2 && col <= 5) || // Línea inferior
            (row == 3 && col == 5) ||            // Vertical derecha (superior)
            (row == 5 && col == 2)) {            // Vertical izquierda (inferior)
          Serial.print("1");
        } else {
          Serial.print("0");
        }
      } else if (level == 3) {
        // Dibujar el "3"
        if ((row == 2 && col >= 2 && col <= 5) || // Línea superior
            (row == 4 && col >= 2 && col <= 5) || // Línea central
            (row == 6 && col >= 2 && col <= 5) || // Línea inferior
            (row == 3 && col == 5) ||            // Vertical derecha (superior)
            (row == 5 && col == 5)) {            // Vertical derecha (inferior)
          Serial.print("1");
        } else {
          Serial.print("0");
        }
      }
    }
    Serial.println(); // Salto de línea al final de cada fila
  }
}


void read_key()
{
  incomingByte = Serial.read();
  switch (incomingByte)
  {
    case 'R':
      key = KEY_RESET;
      break;
    case 'S':
      key = KEY_START;
      break;
    case 'A':
      key = KEY_LEFT;
      break;
    case 'D':
      key = KEY_RIGHT;
      break;
    default:
      key = KEY_VOID;
      break;
  }
}

void state_machine_run(byte *pointerRegMatrix, byte *pointerRegCar, byte *pointerShiftDir)
{
  switch(state)
  {
    case STATE_RESET:
      
      writeResetMatrix(pointerRegMatrix, pointerRegCar);
      PrintALLMatrix(pointerRegMatrix, pointerRegCar);
      i=0;
      level = 0;
      points = 0;
      delay(2000);
      if(key == KEY_START){
        state = STATE_CLEAR;
      }else{
        state = STATE_START;
      }
      break;

    case STATE_START:
      writeStartMatrix(pointerRegMatrix, pointerRegCar);
      PrintALLMatrix(pointerRegMatrix, pointerRegCar);
      delay(2000);
      if(key == KEY_START){
        state = STATE_CLEAR;
      }else{
        state = STATE_RESET;
      }
      break;
    
    case STATE_CLEAR:
      writeClearMatrix(pointerRegMatrix, pointerRegCar);
      //PrintALLMatrix(pointerRegMatrix, pointerRegCar);
      state = STATE_CHECK;
      break;

    case STATE_CHECK:
    {
      if(key = KEY_RESET){
        state = STATE_RESET;
      }
      pointerShiftDir[0] = B00000000;
      switch (status)
      {
        case STATUS_NEXT:
          state = STATE_GO;
          break;
        
        case STATUS_LEVEL:
          state = STATE_SHOW_LEVEL;
          status = STATUS_NEXT;
          break;
        
        case STATUS_WIN:
          state = STATE_WIN;
          break;
        
        case STATUS_LOST:
          state = STATE_LOST;
          break;
        
      }
      break;
    }
  
    case STATE_GO:
      
      pointerShiftDir[0] = B00000000;
      writeCarBase(pointerRegCar, pointerShiftDir);
      writeGoCarsMatrix(pointerRegMatrix);
      PrintALLMatrix(pointerRegMatrix, pointerRegCar);
      checkMatrix(pointerRegMatrix, pointerRegCar);
      delay(delaytime);
      read_key();
      
      if(key == KEY_LEFT){
        state = STATE_LEFT;
        key = KEY_VOID;
        state_machine_run(pointerRegMatrix, pointerRegCar, pointerShiftDir);
      }else if (key == KEY_RIGHT){
        state = STATE_RIGHT;
        key = KEY_VOID;
        state_machine_run(pointerRegMatrix, pointerRegCar, pointerShiftDir);
      }else{
        state = STATE_CHECK;
      }
      PrintALLMatrix(pointerRegMatrix, pointerRegCar);
      break;
    
    case STATE_SHOW_LEVEL:
      writeLevelMatrix(pointerLevelMatrix, level);
      delay(2000);
      writeLevelMatrix(pointerLevelMatrix, 1000);
      state = STATE_CHECK;
      if (status == STATUS_LEVEL){
        status = STATUS_NEXT;
        }
      break;
    
    case STATE_LOST:
      writeLostMatrix(pointerRegMatrix, pointerRegCar);
      PrintALLMatrix(pointerRegMatrix, pointerRegCar);
      delay(2000);
      if(key == KEY_RESET){
        state = STATE_RESET;}
        
      else if(key == KEY_START)
        {state = STATE_RESET;}
      else
        {state = STATE_SHOW_LEVEL;}
      break;
    
    case STATE_WIN:
      writeWinMatrix(pointerRegMatrix, pointerRegCar);
      PrintALLMatrix(pointerRegMatrix, pointerRegCar);
      delay(2000);
      if(key == KEY_RESET)
        state = STATE_CHECK;
      else if(key == KEY_START)
        state = STATE_CHECK;
      else
        {state = STATE_SHOW_LEVEL;}
      break;
    
    case STATE_LEFT:
      pointerShiftDir[0] = B00000001;
      writeCarBase(pointerRegCar, pointerShiftDir);
      pointerShiftDir[0] = B00000000;
      state = STATE_CHECK;
      break;
    
    case STATE_RIGHT:
      pointerShiftDir[0] = B00000010;
      writeCarBase(pointerRegCar, pointerShiftDir);
      pointerShiftDir[0] = B00000000;

      state = STATE_CHECK;
      break;
    
  }
  
}

void loop()
{
  Serial.println("level: " + String(level) + " | " + "points" + String(points) + " | key: " + String(key));
  read_key();
  state_machine_run(pointerRegMatrix,pointerRegCar,pointerShiftDir);
  
  Serial.println("level: " + String(level) + " | " + "points" + String(points) + " | key: " + String(key));
}
