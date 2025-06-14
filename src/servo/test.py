offset = 10
axis = 10
example = []

example = [int(i) for i in range(axis+1)]
example += [int(10) for i in range(5)]
example.append(11)
example += reversed([int(i) for i in range(axis+1)])
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

def displayAngle(corrected, direction):
    if corrected == 0 and direction is True:
        return "360°"  # solo visual
    return f"{corrected}°"


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
                corrected:  {displayAngle(corrected, direction)},\
                turns:      {turns},\
                dir:        {direction}")
        last_corrected = corrected


if __name__ == '__main__':
    main()
