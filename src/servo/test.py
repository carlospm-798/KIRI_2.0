offset = 10
axis = 15
example = []

'''example = [int(i) for i in range(axis+1)]
example += [int(10) for i in range(5)]
example.append(11)
example += reversed([int(i) for i in range(axis+1)])
print(example)'''

example += reversed([int(i) for i in range(axis)])
print(example)

def getAngle(deg):
    return deg - offset

def getCorrectedAngle(deg):
    corrected = deg - offset
    if corrected < 0:
        corrected += 360
    elif corrected >= 360:
        corrected -= 360
    return corrected

def displayAngle(corrected, direction, turns):
    corrected_with_turns = corrected + (turns * 360)

    # Solo visualmente mostrar 360 si está en vuelta 0 y es creciente
    if corrected == 0 and direction is True and turns == 0:
        return "360°"
    
    return f"{corrected_with_turns}°"



def getDirection(current, last):
    if current > last:
        return True
    elif current < last:
        return False
    return None  # sin movimiento

def main():
    last = 0
    turns = 0
    last_corrected = getCorrectedAngle(0)
    for i in example:
        corrected = getCorrectedAngle(i)
        direction = getDirection(corrected, last_corrected)

        if direction is not None:
            if last_corrected == 359 and corrected == 0:
                turns += 1
            elif last_corrected == 0 and corrected == 359:
                turns -= 1

        print(f"current:    {i},\
                angle:      {getAngle(i)},\
                corrected:  {displayAngle(corrected, direction, turns)},\
                turns:      {turns},\
                dir:        {direction}")
        last_corrected = corrected


if __name__ == '__main__':
    main()
