import numpy as np


def metrics(bins, y_truth, y_predict):
    stats = []

    for win_truth, win_predict in zip(y_truth, y_predict):
        truth = np.full(len(bins), False)
        predict = np.full(len(bins), False)
        
        truth[win_truth] = True
        predict[win_predict] = True
        
        tp = sum([int(x and y) for x, y in zip(truth, predict)])
        tn = sum([int(not x and not y) for x, y in zip(truth, predict)])
        fp = sum([int(not x and y) for x, y in zip(truth, predict)])
        fn = sum([int(x and not y) for x, y in zip(truth, predict)])
        
        tpr = tp / (tp + fn)
        tnr = tn / (tn + fp)
        ppv = 0
        if tp + fp != 0:
            ppv = tp / (tp + fp)
        acc = (tp + tn) / (tp + tn + fp + fn)
        fpr = fp / (fp + tn)
            
        stats.append({
            'TP': tp, 'TN': tn, 'FP': fp, 'FN': fn, 
            'TPR': tpr, 'TNR': tnr,  'PPV': ppv,
            'ACC': acc, 'FPR': fpr
        })

    return stats

def metric_macro_average(metrics):
    averages = {
        'TPR': 0, 'TNR': 0,  'PPV': 0, 'ACC': 0, 'FPR': 0
    }
    for win_metrics in metrics:
        for k in averages.keys():
            averages[k] += win_metrics[k]
        
    for k in averages.keys():
        averages[k] /= len(metrics)
        
    return averages


def metric_micro_average(metrics):
    averages = {
        'TP': 0, 'TN': 0, 'FP': 0, 'FN': 0, 
    }
    for win_metrics in metrics:
        for k in averages.keys():
            averages[k] += win_metrics[k]

    tp, tn, fp, fn = (
        averages['TP'], averages['TN'], averages['FP'], averages['FN']
    )
    tpr = tp / (tp + fn)
    tnr = tn / (tn + fp)
    ppv = 0
    if tp + fp != 0:
        ppv = tp / (tp + fp)
    acc = (tp + tn) / (tp + tn + fp + fn)
    fpr = fp / (fp + tn)
    
    return {
        'TPR': tpr, 'TNR': tnr, 'PPV': ppv,
        'ACC': acc, 'FPR': fpr
    }


def evaluation(bins, truth, peaks, mode):
    if mode == 'micro':
        return metric_micro_average(metrics(bins, truth, peaks))
    elif mode == 'macro':
        return metric_macro_average(metrics(bins, truth, peaks))


def grid_search(spectrum, truth, algorithm, mode, *args):
    best = None
    accuracy = 0
    for params in np.array(np.meshgrid(*args)).T.reshape(-1, len(args)):
        m = evaluation(spectrum['bins'], truth, algorithm(spectrum, *params), mode)
        if m['ACC'] > accuracy:
            accuracy = m['ACC']
            best = params
    return best
