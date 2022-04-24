import pandas as pd


def load_dataset(dataset):
    """
    Výstupom je tabuľka s troma atribútmi osí akcelerácie [m/s^2]: x, y, z a časovým indexom t [s]
    """
    PATH = 'datasets/'
    FS_HZ = 500

    measurement = pd.read_csv(PATH + dataset, sep=' ')
    measurement['t'] = measurement.index * (1 / FS_HZ)
    measurement.set_index('t', inplace=True)

    return measurement, FS_HZ


# Odstrániť chybné hodnoty mimo rozsah: 
# vibrations[vibrations['y'].abs() > 2*G]
# measurement[measurement.isnull().any(axis=1)]
# pd.to_numeric(measurement['x']) 
