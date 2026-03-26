int positionShuffle() {
  int randomIndex;

  do {
    randomIndex = random(NUM_COUNT);
  } while (randomIndex == lastPickedIndex);

  int pickedNumber = numbers[randomIndex];
  lastPickedIndex = randomIndex;

  return pickedNumber;
}
