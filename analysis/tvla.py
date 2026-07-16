import sys
import numpy as np
from scipy import stats

def tvla(fixed_file, random_file):
    x_f = np.loadtxt(fixed_file)
    x_r = np.loadtxt(random_file)

    # Identical percentile filtering (1st-99th)
    lo, hi = np.percentile(np.concatenate([x_f, x_r]), [1, 99])
    x_f = x_f[(x_f >= lo) & (x_f <= hi)]
    x_r = x_r[(x_r >= lo) & (x_r <= hi)]

    # Welch's t-test
    t, p = stats.ttest_ind(x_f, x_r, equal_var=False)
    abs_t = abs(t)
    
    # Cohen's d
    s_pooled = np.sqrt(((len(x_f)-1)*x_f.var(ddof=1) + (len(x_r)-1)*x_r.var(ddof=1)) / (len(x_f) + len(x_r) - 2))
    d = (x_f.mean() - x_r.mean()) / s_pooled if s_pooled > 0 else 0.0

    verdict = "FAIL" if abs_t > 4.5 else "PASS"
    print(f"{fixed_file.split('/')[-1]} vs {random_file.split('/')[-1]}: |t|={abs_t:.4f}, d={d:.4f} -> {verdict}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 tvla.py <fixed_trace.csv> <random_trace.csv>")
        sys.exit(1)
    tvla(sys.argv[1], sys.argv[2])