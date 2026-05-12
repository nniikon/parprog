import subprocess
import os
import matplotlib.pyplot as plt
import numpy as np

def compile_c_code():
    subprocess.run(["gcc", "sort.c", "-o", "sort_bin", "-pthread", "-O2"], check=True)
    subprocess.run(["gcc", "ipc.c", "-o", "ipc_bin", "-pthread", "-O2"], check=True)

def run_benchmark(executable, arg):
    result = subprocess.run([f"./{executable}", str(arg)], capture_output=True, text=True)
    data = {}
    for line in result.stdout.strip().split('\n'):
        if '=' in line:
            key, val = line.split('=')
            data[key] = float(val)
    return data

plt.rcParams.update({
    'font.size': 12,
    'axes.facecolor': '#f8f9fa',
    'axes.edgecolor': '#dee2e6',
    'axes.grid': True,
    'grid.color': '#e9ecef',
    'grid.linestyle': '--'
})

def benchmark_sorting():
    print("Запуск бенчмарка сортировки...")
    array_sizes = [500_000, 1_000_000, 2_500_000, 5_000_000, 10_000_000]
    qsort_times = []
    parallel_times = []

    for size in array_sizes:
        print(f"   -> Сортировка {size // 1000000}M элементов...")
        res = run_benchmark("sort_bin", size)
        qsort_times.append(res['QSORT'])
        parallel_times.append(res['PARALLEL'])

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(array_sizes, qsort_times, marker='o', markersize=8, linewidth=2.5, color='#ff6b6b', label='Однопоточный qsort')
    ax.plot(array_sizes, parallel_times, marker='s', markersize=8, linewidth=2.5, color='#4dabf7', label='Параллельный Merge Sort')

    ax.fill_between(array_sizes, qsort_times, parallel_times, color='#e3fafc', alpha=0.5)
    ax.set_title('Масштабируемость алгоритмов сортировки', pad=20, fontsize=16, fontweight='bold', color='#343a40')
    ax.set_xlabel('Размер массива (млн элементов)', labelpad=10, fontsize=14)
    ax.set_ylabel('Время выполнения (секунды)', labelpad=10, fontsize=14)
    ax.set_xticks(array_sizes)
    ax.set_xticklabels([f"{s/1e6:.1f}M" for s in array_sizes])
    ax.legend(loc='upper left', frameon=True, shadow=True, fancybox=True)

    plt.tight_layout()
    plt.savefig('beautiful_sorting.png', dpi=300, bbox_inches='tight')
    print("📸 График сортировки сохранен: beautiful_sorting.png\n")


def benchmark_ipc():
    print("🔄 Запуск бенчмарка IPC (Межпроцессное взаимодействие)...")
    iterations = 500_000
    print(f"   -> Прогон ping-pong теста ({iterations} итераций)...")
    res = run_benchmark("ipc_bin", iterations)

    methods = ['Pipes (через Ядро ОС)', 'Shared Mem (через RAM)']
    times = [res['PIPES'], res['SHM']]
    colors = ['#ff8787', '#63e6be']

    fig, ax = plt.subplots(figsize=(8, 6))
    bars = ax.bar(methods, times, color=colors, width=0.6, edgecolor='#343a40', linewidth=1.5)

    ax.set_title(f'Накладные расходы IPC\n({iterations:,} сообщений "туда-обратно")', pad=20, fontsize=16, fontweight='bold', color='#343a40')
    ax.set_ylabel('Общее время (секунды)', labelpad=10, fontsize=14)

    for i, bar in enumerate(bars):
        yval = bar.get_height()
        us_per_call = (yval / iterations) * 1_000_000
        ax.text(bar.get_x() + bar.get_width()/2, yval + 0.02,
                f"{yval:.3f} сек\n({us_per_call:.2f} µs/вызов)",
                ha='center', va='bottom', fontsize=12, fontweight='bold', color='#495057')

    plt.tight_layout()
    plt.savefig('beautiful_ipc.png', dpi=300, bbox_inches='tight')
    print("📸 График IPC сохранен: beautiful_ipc.png\n")

if __name__ == "__main__":
    compile_c_code()
    benchmark_sorting()
    benchmark_ipc()
    print("🎉 Вся работа выполнена по максимальной красоте! Открой сгенерированные PNG файлы.")
