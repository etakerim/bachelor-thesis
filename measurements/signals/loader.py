import pandas as pd


def load_dataset(source, dataset, G=9.80665):
    """
    Výstupom je tabuľka s troma atribútmi osí akcelerácie [m/s^2]: x, y, z a časovým indexom t [s]
    """
    if source == 'r-das': 
        FS_HZ = 952
        PATH = 'datasets/'

        measurement = pd.read_csv(PATH + dataset, sep='\t', skiprows=2)
        measurement = measurement[['A_X [mg] ', ' A_Y [mg] ', ' A_Z [mg] ']].rename(columns={
            'A_X [mg] ': 'x', ' A_Y [mg] ': 'y', ' A_Z [mg] ': 'z'
        })
        measurement['t'] = measurement.index * (1 / FS_HZ)
        measurement.set_index('t', inplace=True)
        return measurement.transform(lambda v: (v / 1000) * G), FS_HZ   # Premena z mg na m/s^2

    elif source == 'esp':
        FS_HZ = 500
        MG_LSB = 0.061
        PATH = 'datasets/'

        measurement = pd.read_csv(PATH + dataset, sep=' ').rename(columns={
            'A_X[LSB]': 'x', 'A_Y[LSB]': 'y', 'A_Z[LSB]': 'z'
        })
        measurement['t'] = measurement.index * (1 / FS_HZ)
        measurement.set_index('t', inplace=True)
        return measurement.transform(lambda v: ((v * MG_LSB) / 1000) * G), FS_HZ   # Premena z LSB na m/s^2
    
    else:
        raise NotImplementedError('Neplatný názov datasetu')
        
# Odstrániť chybné hodnoty mimo rozsah: 
# vibrations[vibrations['y'].abs() > 2*G]
# measurement[measurement.isnull().any(axis=1)]
# pd.to_numeric(measurement['x']) 
