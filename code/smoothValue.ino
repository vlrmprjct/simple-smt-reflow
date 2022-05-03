const int numReadings = 20;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;

int smoothValue(int value) {
    total = total - readings[readIndex];
    readings[readIndex] = value;
    total = total + readings[readIndex];
    readIndex = readIndex + 1;
    if (readIndex >= numReadings) {
        readIndex = 0;
    }
    average = total / numReadings;
    delay(1);
    return average;
}
