#include <IRremote.h>
#define REDOUTPUT 10
#define GREENOUTPUT 5
#define BLUEOUTPUT 6
#define IR_RECEIVE_PIN 8

typedef struct colour {
    int r;
    int g;
    int b;
}Colour;

Colour allColours[] = {
    {255,0,0  }, { 0,255,0 }, {0,0,255  },
    {255,63,0 }, {0,255,63 }, {63,0,255 },
    {255,127,0}, {0,255,127}, {127,0,255},
    {255,191,0}, {0,255,191}, {191,0,255},
    {255,255,0}, {0,255,255}, {255,0,255}
};

char *allColourIndex[] = {
    "Red", "Green", "Blue", "RG25", "GB25", "BR25", "RG50", "GB50", "BR50", 
    "RG75", "GB75", "BR75", "Yellow", "Cyan", "Purple"
};


char *animationList[] = {
    "constant", "fadeColour", "fadeRainbow", "jumpRainbow",
    "strobeRainbow", "strobeColour"
};

typedef struct LEDSTATE {
   char *animation;
   // Brightness is only when the colour is constant (not for fade)
   float brightness;
   int isOn;
   //For now just make it inbetween 1 and 100
   int ledSpeed;
   float incr;
   Colour colour;
   bool isChooseColour;
   int chooseColourCol;
} LEDSTATE;

Colour currentColour = {0,0,0};
Colour nothing = {0,0,0};
Colour white = {255,255,255};

LEDSTATE globalState = {"constant", 1, 1, 1, 0.01, white, false};

int stateChange = 0;
void showColour(Colour, float, int);
void fadeInOut(Colour, Colour, bool);
void fadeRainbow();
void fadeColour();
void strobeColour();
void strobeRainbow();
void jumpRainbow();
Colour addTwoColours(Colour c1, Colour c2);
Colour colMult(Colour c1, float i);
int currAnimationIndex();
int currColourIndex();
int sameColour(Colour c1, Colour c2);
void checkIR();

void setup() {
    // Open serial communications and wait for port to open:
    pinMode(REDOUTPUT, OUTPUT);
    pinMode(GREENOUTPUT, OUTPUT);
    pinMode(BLUEOUTPUT, OUTPUT);
    Serial.begin(9600);
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver, enable feedback LED, take LED feedback pin from the internal boards definition
}

void loop() {

    checkIR();
    displayState();


    

}

void chooseColour(Colour col) {
    int redVal;
    int greenVal;
    int blueVal;

    // This will either be 1,2 or 3. 1 means red, 2 means blue, 3 means green
    int whichColSelected;


}


void showColour(Colour col, float i, int withState) {
    int redCol = constrain((globalState.colour.r * withState + col.r) * i * globalState.brightness, 0, 255);
    int greenCol = constrain((globalState.colour.g * withState + col.g) * i * globalState.brightness, 0, 255);
    int blueCol = constrain((globalState.colour.b * withState + col.b) * i * globalState.brightness, 0, 255);
    analogWrite(REDOUTPUT, redCol);
    analogWrite(GREENOUTPUT, greenCol);
    analogWrite(BLUEOUTPUT, blueCol);
    checkIR();
}

void fadeInOut(Colour constCol, Colour changeCol, bool fadeIn) {
    for (float i=0; i < 1; i += globalState.incr) {
        Colour fadeCol;
        if (fadeIn) {
            fadeCol = colMult(changeCol, i);
            showColour(addTwoColours(constCol,fadeCol), 1, 0);
        } else {
            fadeCol = colMult(changeCol, 1 - i);
            showColour(addTwoColours(constCol,fadeCol), 1, 0);
        }
        if (stateChange) {
            return;
        }
        delay(100/globalState.ledSpeed);
    }
}

void fadeRainbow() {
    Colour red = {255,0,0};
    Colour green = {0,255,0};
    Colour blue = {0,0,255};
    showColour(red, 1, 0);
    if (stateChange) {
        return;
    }
    fadeInOut(red, green, 1);
    if (stateChange) {
        return;
    }
    fadeInOut(green, red, 0);
    if (stateChange) {
        return;
    }
    fadeInOut(green, blue, 1);
    if (stateChange) {
        return;
    }
    fadeInOut(blue, green, 0);
    if (stateChange) {
        return;
    }
    fadeInOut(blue, red, 1);
    if (stateChange) {
        return;
    }
    fadeInOut(red, blue, 0);
}

void fadeColour() {
    Serial.println("hello there");
    fadeInOut(nothing, globalState.colour, 1);
    if (stateChange) {
        return;
    }
    fadeInOut(nothing, globalState.colour, 0);
    if (stateChange) {
        return;
    }
}

void strobeColour() {
    // Make sure LEDSTATE has colour
    //
    showColour(nothing, 1, 1);
    delay(20);
    showColour(nothing, 1, 0);
    delay(2000/globalState.ledSpeed);
}

void strobeRainbow() {
    for (int i=0; i < 15; i++) {
        showColour(allColours[i], 1, 0);
        delay(20);
        showColour(nothing, 1, 0);
        delay(2000/globalState.ledSpeed);
        if (stateChange) {
            return;
        }
    }
}

void jumpRainbow() {
    for (int i=0; i < 15; i++) {
        showColour(allColours[i], 1, 0);
        if (stateChange) {
            return;
        }
        delay(10000/globalState.ledSpeed);
    }
}

Colour addTwoColours(Colour c1, Colour c2) {
  Colour newCol;
  int newRed = c1.r + c2.r;
  int newGreen = c1.g + c2.g;
  int newBlue = c1.b + c2.b;
  newCol = {newRed, newGreen, newBlue};
  return newCol;
}

Colour colMult(Colour c1, float i) {
    Colour newCol;
    int newRed = c1.r * i * i;
    int newGreen = c1.g * i * i;
    int newBlue = c1.b * i * i;
    newCol = {newRed, newGreen, newBlue};
    return newCol;
}

void checkIR() {
    if (IrReceiver.decode()) {
        //Looks at the sent ir message
        Serial.println(IrReceiver.decodedIRData.decodedRawData);
        switch (IrReceiver.decodedIRData.decodedRawData) {
            // Source (turn off)
            case 165:
                if (globalState.isOn) {
                    globalState.isOn = 0;
                    stateChange = 1;
                }
                break;
            // On/Off (turn on)
            case 149:
                if (!globalState.isOn) {
                    globalState.isOn = 1;
                    stateChange = 1;
                }
                break;
            // TrackID (fadeRainbow)
            case 3454:
                if (globalState.animation != "fadeRainbow") {
                    globalState.animation = "fadeRainbow";
                    stateChange = 1;
                }
                break;
                
            // Go back (decrease brightness)
            case 19388:
                globalState.brightness = constrain(globalState.brightness - 0.1, 0.1, 1);
                break;
            // Pause (rotate constant colour backwards)
            case 19353:
                if (globalState.animation != "constant") {
                    globalState.animation = "constant";
                    globalState.colour = allColours[0];
                } else {
                    int newColourIndex = currColourIndex() - 3;
                    if (newColourIndex < 0) {
                        newColourIndex += 14;
                    }   
                    globalState.colour = allColours[newColourIndex];
                    stateChange = 1;

                }
                break;
            // Go forward (increase brightness)
            case 19389:
                globalState.brightness = constrain(globalState.brightness + 0.1, 0.1, 1);
                break;
            // Manual (jumpRainbow)
            case 3451:
                if (globalState.animation != "jumpRainbow") {
                    globalState.animation = "jumpRainbow";
                    stateChange = 1;
                }
                break;
                
            // Rewind (reduce speed)
            case 19355:
                // MAYBE MAKES IT TOO FAST
                // if (globalState.incr > 0.01) {
                //     globalState.incr = constrain(globalState.incr - 0.15, 0.01, 1);
                // } else {
                //     globalState.ledSpeed = constrain(globalState.ledSpeed - 10, 1, 100);
                // }
                globalState.ledSpeed = constrain(globalState.ledSpeed - 1, 1, 100);

                break;
            // Play (rotate constant colour forwards)
            case 19354:
                Serial.println("---------------------------------");
                if (globalState.animation != "constant") {
                    globalState.animation = "constant";
                    globalState.colour = allColours[0];
                } else {
                    int newColourIndex = currColourIndex() + 3;
                    if (newColourIndex >= 15) {
                        newColourIndex += 1;
                    }
                    newColourIndex %= 15;
                    globalState.colour = allColours[newColourIndex];
                }
                stateChange = 1;
                break;
            // Fast forward (increase speed)
            case 19356:
                // MAYBE MAKES IT TOO FAST
                // if (globalState.ledSpeed < 100) {
                //     globalState.ledSpeed = constrain(globalState.ledSpeed + 10, 1, 100);
                // } else {
                //     globalState.incr = constrain(globalState.incr + 0.15, 0.01, 1);
                // }
                globalState.ledSpeed = constrain(globalState.ledSpeed + 1, 1, 100);

                break;
            // DIGITAL/ANALOG (fadeColour) 
            case 15245:
                if (globalState.animation != "fadeColour") {
                    globalState.animation = "fadeColour";
                    stateChange = 1;
                }
                break;
                
            // Half box (strobeColour)    
            case 21111: 
                if (globalState.animation != "strobeColour") {
                    globalState.animation = "strobeColour";
                    stateChange = 1;
                }
                break;
                
            // INTERNET/VIDEO (strobeRainbow)
            case 3449:
                if (globalState.animation != "strobeRainbow") {
                    globalState.animation = "strobeRainbow";
                    stateChange = 1;
                } 
                break;
            // Red
            case 19365:
                if (!globalState.isChooseColour) {
                    globalState.animation = animationList[0];       // Constant
                    globalState.colour = {255,0,0};                 // Red
                } else {
                    globalState.chooseColourCol = 1;
                }
                stateChange = 1;
                break;
            // Green 
            case 19366:
                if (!globalState.isChooseColour) {
                    globalState.animation = animationList[0];       // Constant
                    globalState.colour = {0,255,0};                 // Green
                } else {
                    globalState.chooseColourCol = 2;
                }
                stateChange = 1;
                break;
            // Yellow 
            case 19367:
                globalState.animation = animationList[0];       // Constant
                globalState.colour = {255,255,255};             // White
                stateChange = 1;
                break;
            // Blue 
            case 19364:
                if (!globalState.isChooseColour) {
                    globalState.animation = animationList[0];       // Constant
                    globalState.colour = {0,0,255};                 // Blue
                } else {
                    globalState.chooseColourCol = 3;
                }
                stateChange = 1;
                break;
            // Middle circle thing
            case 229:
                globalState.isChooseColour = 1;
                stateChange = 1;
                break;
            // Up arrow on middle circle
            case 244:
                Serial.print("old col is");
                Serial.print("r");
                Serial.print(globalState.colour.r);
                Serial.print("g");
                Serial.print(globalState.colour.g);
                Serial.print("b");
                Serial.print(globalState.colour.b);
                if (globalState.isChooseColour) {
                    if (globalState.chooseColourCol == 1){
                        globalState.colour = {globalState.colour.r + 5, globalState.colour.g, globalState.colour.b};
                    }
                    else if (globalState.chooseColourCol == 2){
                        globalState.colour = {globalState.colour.r, globalState.colour.g + 5, globalState.colour.b};
                    }
                    else if (globalState.chooseColourCol == 3){
                        globalState.colour = {globalState.colour.r, globalState.colour.g, globalState.colour.b + 5};
                    }
                    stateChange = 1;
                }
                Serial.print("new col is");
                Serial.print("r");
                Serial.print(globalState.colour.r);
                Serial.print("g");
                Serial.print(globalState.colour.g);
                Serial.print("b");
                Serial.print(globalState.colour.b);
                break;

            // Down arrow on middle circle
            case 245:
                if (globalState.isChooseColour) {
                    if (globalState.chooseColourCol == 1){
                        globalState.colour = {globalState.colour.r - 5, globalState.colour.g, globalState.colour.b};
                    }
                    else if (globalState.chooseColourCol == 2){
                        globalState.colour = {globalState.colour.r, globalState.colour.g - 5, globalState.colour.b};
                    }
                    else if (globalState.chooseColourCol == 3){
                        globalState.colour = {globalState.colour.r, globalState.colour.g, globalState.colour.b - 5};
                    }
                    stateChange = 1;
                }
                break;
            // Home button
            case 224:
                globalState.isChooseColour = false;
                stateChange = 1;
                break;

        }
        IrReceiver.resume(); // Continue receiving
    }

}

void displayState() {
//    Serial.println(globalState.isOn);
//    Serial.println(globalState.colour.r);
    stateChange = 0;
    if (!globalState.isOn) {
        showColour(nothing, 1, 0);
        return;
    }
    if (globalState.animation == "constant") {
        showColour(nothing, 1, 1);
    } else if (globalState.animation == "fadeColour") {
        fadeColour();
    } else if (globalState.animation == "fadeRainbow") {
        fadeRainbow();
    } else if (globalState.animation == "jumpRainbow") {
        jumpRainbow();
    } else if (globalState.animation == "strobeColour") {
        strobeColour();
    } else if (globalState.animation == "strobeRainbow") {
        strobeRainbow();
    }
}

int currAnimationIndex() {
    for (int i=0; i < 6; i++) {
        Serial.println(globalState.animation);
        Serial.println(animationList[i]);
        Serial.println(globalState.animation == animationList[i]);
        if (globalState.animation == animationList[i]) {
            return i;
        }
    }
    return 0;
}

int currColourIndex() {
    for (int i=0; i < 15; i++) {
        if (sameColour(globalState.colour,allColours[i])) {
            return i;
        }
    }
    return 0;
}

int sameColour(Colour c1, Colour c2) {
    if (c1.r != c2.r || c1.g != c2.g || c1.b != c2.b) {
        return 0;
    } else {
        return 1;
    }
}
