from graphs.gen import *
import argparse

import matplotlib as mpl
mpl.use("pgf")
import matplotlib.pyplot as plt
import numpy as np

import subprocess

pgf_with_latex = {                      # setup matplotlib to use latex for output
    "pgf.texsystem": "pdflatex",        # change this if using xetex or lautex
    "text.usetex": True,                # use LaTeX to write all text
    "font.family": "DejaVu Sans",
    "font.serif": [],                   # blank entries should cause plots 
    "font.sans-serif": [],              # to inherit fonts from the document
    "font.monospace": [],
    "axes.labelsize": 15,
    "font.size": 15,
    "legend.fontsize": 15,               # Make the legend/label fonts 
    "xtick.labelsize": 15,               # a little smaller
    "ytick.labelsize": 15,
    "figure.figsize": [0.9, 0.9],     # default fig size of 0.9 textwidth
    "pgf.preamble": [
        r"\usepackage[utf8]{inputenc}",    # use utf8 input and T1 fonts 
        r"\usepackage[T1]{fontenc}",        # plots will be generated 
        ]                                   # using this preamble
    }
mpl.rcParams.update(pgf_with_latex)


def retvals(out):
    x = out.decode("utf-8").strip().split(",")
    return int(x[0]), float(x[1])

def run(cmd, timeout=None):
    x = None
    outed = False
    try:
        x = subprocess.check_output(cmd.split(" "), timeout=timeout)
    except subprocess.TimeoutExpired as e:
        outed = True
        x = e.output
    if x is None:
        return 0, 10000, 1000        
    lines = x.decode("utf-8").split("\n")
    x = lines[-2].strip().split(",")
    if outed:
        if len(x) == 3:
            return (int(x[0]), int(x[1]), float(x[2]))
        else:
            return (0, int(x[0]), float(x[1]))
    else:
        return (int(x[0]), float(x[1]))

def runGlucoseUno(filename, score, timeout=None):
    return run(f"./run -f {filename} -I {score}", timeout=timeout)

def runGlucose(filename, inc=0, timeout=None):
    return run(f"./run -f {filename} -I {inc}", timeout=timeout)

def runGecode(filename, timeout=None):
    return run(f"./run -f {filename} -g -p", timeout=timeout)

def exp1(n):
    f = "graphs/dag_10_15_rdm"
    G = gen_WDiGraph(10, 15, 100, False)
    netz_graphto(f, G)
    gen_beautiful_graph(G, f, "dot", canvax=6, canvay=6)
    timeout = 15
    
    inc = []
    nai = []
    gec = []

    sc = -1

    for i in range(0, n):
        sco, tim = runGlucose(f, 1)
        if sc == -1:
            sc = sco
        if sc != sco:
            print(i, "inc disagree with itself??", sc, "!=", sco)
        inc.append(tim)
        sco, tim = runGlucose(f, 0)
        if sc != sco:
            print(i, "noinc", sc, "!=", sco)
        nai.append(tim)
        sco, tim = runGecode(f)
        if sc != sco:
            print(i, "gecode", sc, "!=", sco)
        gec.append(tim)
        print(i, "done", inc[i-1], nai[i-1], gec[i-1])

    print(inc)
    print(nai)
    print(gec)


def exp2(n):
    f = "graphs/temp"
    nn = 10
    m = 15
    maxw = 100
    optimal = 0
    timedout = 0
    vi, v, vg = [], [], []
    for i in range(0, n):
        ff = f + str(i)
        G = gen_WDiGraph(nn, m, maxw, False)
        netz_graphto(ff, G)
        sgi, tgi = runGlucose(ff, 1)
        print("GI=", sgi, "@", tgi)
        sg, tg = runGlucose(ff, 0)
        print("G =", sg, "@", tg)
        res = runGecode(ff)
        sge ,tge = 0,None
        vi.append(tgi)
        v.append(tg)
        if len(res) == 2:
            sge, tge = res[0], res[1]
            optimal += 1
            vg.append(tge)
            print("GE=", sge, "@", tge)
        else:
            sge, tge = res[1], res[2]
            if sge == sg:
                optimal += 1
                vg.append(tge)
            timedout += 1
            print("GE=", sge, "@", tge, " (timed out)")
        increthree(tgi, tg, tge, "exp2.csv")
        print(timedout/(i+1)*100, "% of timeouts")
        print(optimal/(i+1)*100, "% of optimal for gecode")

    print(timedout/n*100, "% of timeouts")
    print(optimal/n*100, "% of optimal for gecode")
    #threevars(vi, v, vg, "exp2.csv")

def jarte(a, v):
    if len(a) == 2:
        v.append(a[1])
    else:
        v.append(-1)

def outed(a):
    if len(a) == 2:
        return False
    return True

def exp3(n):
    f = "graphs/temp"
    nn = 30
    m = 0
    maxw = 100
    timeout = 0.5

    vgi, vg, vge = [], [], []
    for i in range(0, nn+1):
        tvgi, tvg, tvge = 0,0,0
        ff = f + str(i)
        print("m=", m, "ff=", ff)
        for j in range(0, n):
            G = gen_WDiGraph(nn, m, maxw, False)
            netz_graphto(ff, G)
            tvgi += outed(runGlucose(ff, 1, timeout))
            tvg += outed(runGlucose(ff, 0, timeout))
            tvge += outed(runGecode(ff, timeout))
        vgi.append(tvgi)
        vg.append(tvg)
        vge.append(tvge)
        print(vgi[-1], vg[-1], vge[-1])

        m += 1
        anyvar([vgi, vg, vge], "exp3.csv")

    anyvar([vgi, vg, vge], "exp3.csv")

def graphexp3():
    x = readanyvar("exp3.csv")
    print(x)
    gi = np.asarray(x[0])/50*100
    g = np.asarray(x[1])/50*100
    ge = np.asarray(x[2])/50*100
    X = np.arange(31)
    fig, ax = plt.subplots(figsize=(7,3))
    ax.set_title("Timeout % on random DAG n=10")
    ax.set_ylabel("% of timeout(5s)")
    ax.set_xlabel("m (number of edges)")
    ax.plot(X, gi, 'r', c=(1,0,0), label="Incremental")
    ax.plot(X, g, 'r', c=(0.2,0.7,0), label="Naive")
    ax.plot(X, ge, 'r', c=(0.2,0.2,0.8) ,label="gecode")
    plt.legend()

    plt.savefig('exp3.pdf',
     bbox_inches = 'tight',
        pad_inches = 0)

def graphexp2():
    f = "exp2.csv"
    a,b,c = [], [], []
    try:
        with open(f, "r") as ff:
            s = ff.read().split("\n")
            ssa = s[0].split(",")
            ssb = s[1].split(",")
            ssc = s[2].split(",")
            a = [float(x) for x in ssa]
            b = [float(x) for x in ssb]
            c = [float(x) for x in ssc]
    except:
        print("couldn't find file", f)
        return
    fig, ax = plt.subplots(figsize=(4,3))
    ax.yaxis.grid(True)
    ax.set_title("Time on random DAG n=10 m=15")
    #ax.set_ylim(0, 200)
    #ax.set_xlim(0, 45)
    ax.set_ylabel("Time (ms)")
    #ax.set_xlabel('Nombre de points moyen')
    Y = [a, b, c]
    ax.boxplot(Y,showfliers=False, showmeans=False, widths=0.2)
    plt.setp(ax, xticks=[1,2,3],
             xticklabels=["Incremental", "Naive", "Gecode"])
    plt.show()
    plt.savefig('exp2.pdf',
     bbox_inches = 'tight',
        pad_inches = 0)

def exp4(n):
    f = "graphs/temp"
    fn = "exp4.csv"
    nn = 7
    m = 15
    maxw = 100
    timeout = 0.5

    vgi, vg, vge = [], [], []
    for i in range(10, 100):
        tvgi, tvg, tvge = 0,0,0
        ff = f + str(i)
        print("nn=", nn, "ff=", ff)
        for j in range(0, n):
            G = gen_WDiGraph(nn, m, maxw, False)
            netz_graphto(ff, G)
            tvgi += outed(runGlucose(ff, 1, timeout))
            tvg += outed(runGlucose(ff, 0, timeout))
            tvge += outed(runGecode(ff, timeout/4))
        vgi.append(tvgi)
        vg.append(tvg)
        vge.append(tvge)
        print(vgi[-1], vg[-1], vge[-1])

        nn += 1
        anyvar([vgi, vg, vge], fn)

    anyvar([vgi, vg, vge], fn)

def graphexp4():
    fn = "exp4"
    x = readanyvar(fn + ".csv")
    gi = np.asarray(x[0])/50*100
    g = np.asarray(x[1])/50*100
    ge = np.asarray(x[2])/50*100
    X = np.arange(7, 76)
    fig, ax = plt.subplots(figsize=(7,3))
    ax.set_title("Timeout % on random DAG with 15 edges")
    ax.set_ylabel("% of timeout(5s)")
    ax.set_xlabel("n (number of tasks)")
    ax.plot(X, gi, 'r', c=(1,0,0), label="Incremental")
    ax.plot(X, g, 'r', c=(0.2,0.7,0), label="Naive")
    ax.plot(X, ge, 'r', c=(0.2,0.2,0.8) ,label="gecode")
    plt.legend()

    plt.savefig(fn + ".pdf",
     bbox_inches = 'tight',
        pad_inches = 0)

def exp5(n):
    f = "graphs/temp"
    fn = "exp5.csv"
    nn = 15
    m = 27
    maxw = 100
    timeout = None

    a = []
    ff = f
    G = gen_WDiGraph(nn, m, maxw, False)
    netz_graphto(ff, G)
    last = 0
    opti = 0
    for score in range(2, nn*maxw):
        x = runGlucoseUno(ff, score, timeout=None)
        if x[0] == 1 and last == 0:
            opti = score
        print("score=", score, x)
        a.append(x[1])

    print("score ===", opti)
    anyvar([a], fn)

def graphexp5():
    fn = "exp5"
    x = readanyvar(fn + ".csv")
    gi = np.asarray(x[0])
    X = np.arange(2, 150)
    fig, ax = plt.subplots(figsize=(7,3))
    ax.set_title("Time in function of score constraint")
    ax.set_ylabel("Time (ms)")
    ax.set_xlabel("Score")
    ax.plot(X, gi, 'r', c=(1,0,0), label="Incremental")
    #plt.legend()

    plt.savefig(fn + ".pdf",
     bbox_inches = 'tight',
        pad_inches = 0)


def increthree(a, b, c, f):
    sssa = []
    sssb = []
    sssc = []
    try:
        with open(f, "r") as ff:
            s = ff.read().split("\n")
            ssa = s[0].split(",")
            ssb = s[1].split(",")
            ssc = s[2].split(",")
            sssa = [float(x) for x in ssa]
            sssb = [float(x) for x in ssb]
            sssc = [float(x) for x in ssc]
            a.extend(sssa)
            b.extend(sssb)
            c.extend(sssc)
    except:
        pass
    if a is not None:
        sssa.append(a)
    if b is not None:
        sssb.append(b)
    if c is not None:
        sssc.append(c)
    sa = ",".join([str(i) for i in sssa])
    sb = ",".join([str(i) for i in sssb])
    sc = ",".join([str(i) for i in sssc])
    s = "\n".join([sa, sb, sc])
    write(s, f)


def anyvar(l, f, delete=False):
    print("printing to ", f)
    ss = []
    for vx in l:
        ss.append(",".join([str(i) for i in vx]))
    s = "\n".join(ss)
    write(s, f)

def readanyvar(f):
    l = []
    with open(f, "r") as ff:
        s = ff.read().split("\n")
        for line in s:
            l.append([float(x) for x in line.split(",")])
    return l

def threevars(a, b, c, f, delete=False):
    if delete:
        pass
    else:
        try:
            with open(f, "r") as ff:
                s = ff.read().split("\n")
                ssa = s[0].split(",")
                ssb = s[1].split(",")
                ssc = s[2].split(",")
                sssa = [float(x) for x in ssa]
                sssb = [float(x) for x in ssb]
                sssc = [float(x) for x in ssc]
                a.extend(sssa)
                b.extend(sssb)
                c.extend(sssc)
        except:
            pass
    sa = ",".join([str(i) for i in a])
    sb = ",".join([str(i) for i in b])
    sc = ",".join([str(i) for i in c])
    s = "\n".join([sa, sb, sc])
    write(s, f)

def x():
    f = "graphs/temp"
    #G = tograph_netz(f)
    #G = gen_netz(200, 3, False, 100)
    G = gen_WDiGraph(9, 36, 10, False)
    netz_graphto(f, G)
    gen_beautiful_graph(G, f, "dot", canvax=8, canvay=8)
    timeout=5
    print(runGlucose(f, 0, timeout))
    print(runGlucose(f, 1, timeout))
    print(runGecode(f, timeout))

def write(s, fil):
    with open(fil, "w") as f:
        f.write(s)

def graphexp1(inc, nai, gec):
    fig, ax = plt.subplots(figsize=(7,3))
    ax.yaxis.grid(True)
    ax.set_title("Variance measure")
    #ax.set_ylim(0, 200)
    #ax.set_xlim(0, 45)
    ax.set_ylabel("Time (ms)")
    #ax.set_xlabel('Nombre de points moyen')
    Y = [inc, nai, gec]
    ax.boxplot(Y,showfliers=False, showmeans=False, widths=0.2)
    plt.setp(ax, xticks=[1,2,3],
             xticklabels=["Glucose Incremental", "Glucose Naive", "Gecode"])
    plt.show()
    plt.savefig('exp1.pdf',
     bbox_inches = 'tight',
        pad_inches = 0)


def exp6(n):
    f = "graphs/temp"
    fn = "exp6.csv"
    nn = 7
    m = 15
    maxw = 100
    timeout = 0.5

    vgi, vg, vge = [], [], []
    for i in range(0, 100):
        tvgi, tvg, tvge = 0,0,0
        ff = f + str(i)
        print("nn=", nn, "ff=", ff)
        for j in range(0, n):
            G = gen_WWDiGraph(nn, m, maxw, i)
            netz_graphto(ff, G)
            tvgi += outed(runGlucose(ff, 1, timeout))
            tvg += outed(runGlucose(ff, 0, timeout))
            tvge += outed(runGecode(ff, timeout/4))
        vgi.append(tvgi)
        vg.append(tvg)
        vge.append(tvge)
        print(vgi[-1], vg[-1], vge[-1])

        nn += 1
        anyvar([vgi, vg, vge], fn)

    anyvar([vgi, vg, vge], fn)

def graphexp6():
    fn = "exp6"
    x = readanyvar(fn + ".csv")
    gi = np.asarray(x[0])/50*100
    g = np.asarray(x[1])/50*100
    ge = np.asarray(x[2])/50*100
    X = np.arange(7, 76)
    fig, ax = plt.subplots(figsize=(7,3))
    ax.set_title("Timeout % on random DAG with 15 edges")
    ax.set_ylabel("% of timeout(5s)")
    ax.set_xlabel("n (number of tasks)")
    ax.plot(X, gi, 'r', c=(1,0,0), label="Incremental")
    ax.plot(X, g, 'r', c=(0.2,0.7,0), label="Naive")
    ax.plot(X, ge, 'r', c=(0.2,0.2,0.8) ,label="gecode")
    plt.legend()

    plt.savefig(fn + ".pdf",
     bbox_inches = 'tight',
        pad_inches = 0)


parser = argparse.ArgumentParser()
parser.add_argument("exp", type=int)
parser.add_argument("graph", type=int)
parser.add_argument("n", type=int)
args = parser.parse_args()
p = args.exp
g = args.graph
n = args.n
if p == 2:
    if g:
        graphexp2()
    else:
        exp2(n)
elif p == 4:
    if g:
        graphexp4()
    else:
        exp4(n)
elif p == 3:
    if g:
        graphexp3()
    else:
        exp3(n)
elif p == 5:
    if g:
        graphexp5()
    else:
        exp5(n)
elif p == 6:
    if g:
        graphexp6()
    else:
        exp6(n)
elif p == 1:
    if g:
        inc = [38.89, 41.55, 35.83, 32.05, 32.1, 33.24, 30.93, 31.08, 56.5, 38.41, 33.79, 35.72, 36.41, 37.28, 36.32, 36.65, 34.3, 36.68, 36.29, 53.26, 38.82, 32.95, 34.07, 36.04, 33.03, 35.87, 40.07, 38.16, 54.85, 30.34, 44.05, 39.41, 35.38, 39.28, 34.01, 32.47, 55.76, 38.1, 34.57, 33.12, 44.68, 35.18, 34.1, 33.06, 35.43, 33.8, 37.66, 35.03, 35.86, 34.64, 36.11, 34.12, 38.19, 38.12, 50.9, 36.64, 36.97, 37.81, 36.9, 31.93, 32.79, 34.04, 94.3, 32.15, 38.67, 33.24, 50.73, 33.39, 37.45, 40.51, 61.67, 33.37, 31.63, 50.3, 31.64, 36.12, 32.35, 37.85, 33.28, 32.45, 34.74, 30.99, 41.41, 33.34, 38.8, 36.95, 32.52, 44.75, 31.77, 36.66, 39.28, 69.54, 35.62, 35.0, 43.43, 37.61, 37.88, 38.09, 63.74, 39.14]
        nai = [106.19, 103.08, 93.72, 93.14, 108.07, 88.0, 83.51, 90.8, 117.61, 98.73, 101.95, 96.42, 92.74, 97.29, 96.04, 127.93, 94.66, 100.34, 96.07, 89.09, 98.69, 95.22, 144.01, 102.03, 102.38, 99.41, 103.66, 100.35, 100.36, 113.39, 110.65, 101.02, 89.25, 88.68, 92.6, 88.9, 174.9, 91.89, 80.85, 85.23, 122.58, 93.28, 90.96, 89.04, 83.62, 87.13, 87.29, 89.31, 86.64, 90.21, 100.51, 119.82, 89.37, 88.07, 87.59, 94.57, 88.94, 86.06, 94.97, 87.27, 155.73, 87.26, 129.41, 94.29, 95.89, 88.66, 90.11, 96.71, 92.78, 112.11, 140.41, 90.66, 94.98, 102.17, 88.62, 87.23, 118.67, 101.67, 89.48, 91.32, 199.39, 90.05, 88.38, 90.14, 94.41, 92.67, 93.47, 175.41, 88.75, 103.89, 120.69, 97.08, 100.31, 109.27, 195.77, 88.02, 98.33, 105.5, 89.52, 96.62]
        gec = [152.73, 141.38, 124.73, 126.77, 136.61, 218.48, 126.74, 149.14, 128.21, 155.04, 145.85, 176.01, 141.4, 147.45, 120.44, 136.63, 142.89, 147.39, 160.61, 144.86, 223.04, 142.19, 148.1, 145.96, 150.85, 153.49, 143.34, 199.56, 127.58, 152.13, 160.69, 149.61, 134.12, 134.86, 134.69, 143.34, 141.58, 155.75, 130.42, 132.16, 136.97, 131.24, 127.88, 227.57, 136.26, 145.29, 133.41, 144.76, 219.86, 141.68, 128.14, 136.1, 129.6, 197.26, 140.51, 142.83, 132.84, 134.9, 152.72, 138.62, 153.67, 129.35, 128.9, 134.56, 134.77, 137.94, 135.32, 142.56, 215.47, 139.67, 126.96, 131.23, 146.44, 219.76, 134.28, 137.8, 154.27, 126.78, 134.61, 130.71, 143.6, 131.97, 132.27, 143.38, 143.23, 137.12, 128.6, 144.96, 143.25, 156.15, 155.07, 147.79, 149.01, 172.02, 140.75, 130.44, 165.22, 132.52, 152.32, 134.94]
        graphexp1(inc, nai, gec)
    else:
        exp1(100)
