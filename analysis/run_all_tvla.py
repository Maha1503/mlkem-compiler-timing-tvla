import os
import glob
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
    return abs_t, d, verdict

if __name__ == "__main__":
    trace_dir = "traces"
    # Find all random traces (m0)
    random_files = glob.glob(os.path.join(trace_dir, "*_m0.csv"))
    
    with open("results.csv", "w") as f_out:
        f_out.write("Primitive,Compiler,Opt,Verdict,|t|,Cohen_d\n")
        
        for rand_file in random_files:
            # Derive the fixed file name
            fixed_file = rand_file.replace("_m0.csv", "_m1.csv")
            
            if os.path.exists(fixed_file):
                # Extract metadata from filename (e.g., traces/gcc_O3_p0_m0.csv)
                basename = os.path.basename(rand_file)
                parts = basename.replace(".csv", "").split("_")
                
                compiler = parts[0]
                opt = parts[1]
                prim = parts[2]
                
                try:
                    abs_t, d, verdict = tvla(fixed_file, rand_file)
                    print(f"Analyzing {compiler} {opt} {prim}: |t|={abs_t:.2f} -> {verdict}")
                    f_out.write(f"{prim},{compiler},{opt},{verdict},{abs_t:.4f},{d:.4f}\n")
                except Exception as e:
                    print(f"Error processing {basename}: {e}")

    print("\n=== TVLA Results saved to results.csv ===")