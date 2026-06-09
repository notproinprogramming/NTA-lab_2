#!/usr/bin/env python3
import subprocess
import re
import sys
import time

DOCKER_IMAGE = "oyakymchuk/nta_cp2_helper:latest"
PROG        = "./prog"
DIGITS_RANGE = range(3, 16)   # 3..15 включно
RUNS_PER_CASE = 5
TIMEOUT_SEC   = 290           # трохи менше ніж 5 хв

# патерн: число^x ≡ число (mod число)
PROBLEM_RE = re.compile(r"(\d+)\^x\s*.{0,3}\s*(\d+)\s*\(mod\s*(\d+)\)")


bf_timed_out = False  # як тільки брут форс не вклався - більше не запускаємо
ph_timed_out = False

def solve(alpha: int, beta: int, p: int, method: int) -> tuple[int, float]:
    """Викликає ./prog <method> alpha beta p, повертає (x, ms)"""
    global bf_timed_out, ph_timed_out

    # пропускаємо брутфорс або СПГ, якщо вже знаємо що вони не встигнуть за 5 хвилин
    if method == 1 and bf_timed_out:
        return -1, float("inf")
    if method == 2 and ph_timed_out:
        return -1, float("inf")

    t0 = time.perf_counter()
    try:
        result = subprocess.run(
            [PROG, str(method), str(alpha), str(beta), str(p)],
            capture_output=True, text=True, timeout=TIMEOUT_SEC
        )
    except subprocess.TimeoutExpired:
        ms = (time.perf_counter() - t0) * 1000
        if method == 1:
            bf_timed_out = True
            print(f"\n  [BF TIMEOUT] brute force won't run for any further cases of this task type", file=sys.stderr)
        if method == 2:
            ph_timed_out = True
            print(f"\n  [PH TIMEOUT] pohlig-hellman won't run for any further cases of this task type", file=sys.stderr)

        return -1, ms
    ms = (time.perf_counter() - t0) * 1000

    m = re.search(r"x\s*=\s*(\d+)", result.stdout)
    x = int(m.group(1)) if m else -1
    return x, ms


def generate_and_solve(problem_type: int, digits: int) -> dict | None:
    """
    Запускає docker, отримує задачу, розв'язує обома методами.
    Повертає dict з часами або None якщо щось пішло не так.
    """
    # подаємо: вибір типу, кількість цифр, відповідь (0 - просто щоб програма не зависла), вихід
    # спочатку генеруємо задачу без відповіді щоб не чекати таймер

    # перший запуск: отримуємо задачу
    proc = subprocess.Popen(
        ["docker", "run", "--rm", "-i", DOCKER_IMAGE],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # чекаємо на рядок з задачею, подаємо вибір меню і кількість цифр
    # після появи рядку з "^x" відразу подаємо відповідь і виходимо
    output = ""
    alpha = beta = p = -1

    try:
        # подаємо вибір і кількість цифр
        proc.stdin.write(f"{problem_type}\n{digits}\n")
        proc.stdin.flush()

        # читаємо поки не знайдемо задачу
        deadline = time.time() + 30
        while time.time() < deadline:
            line = proc.stdout.readline()
            if not line:
                break
            output += line
            m = PROBLEM_RE.search(line)
            if m:
                alpha, beta, p = int(m.group(1)), int(m.group(2)), int(m.group(3))
                break

        if alpha < 0:
            proc.stdin.write("0\n3\n")
            proc.stdin.flush()
            proc.wait(timeout=10)
            print(f"  [WARN] could not parse problem (type={problem_type}, digits={digits})",
                  file=sys.stderr)
            return None

        # розв'язуємо обома методами поки docker ще чекає
        x_bf, ms_bf = solve(alpha, beta, p, method=1)
        x_ph, ms_ph = solve(alpha, beta, p, method=2)

        # відповідь беремо з СПГ, fallback - брут форс
        answer = x_ph if x_ph >= 0 else x_bf

        # відправляємо відповідь і виходимо
        proc.stdin.write(f"{answer}\n3\n")
        proc.stdin.flush()
        proc.wait(timeout=15)

    except Exception as e:
        proc.kill()
        print(f"  [ERR] {e}", file=sys.stderr)
        return None

    return {
        "alpha": alpha, "beta": beta, "p": p,
        "ms_bf": ms_bf, "ms_ph": ms_ph,
        "ok_bf": x_bf >= 0, "ok_ph": x_ph >= 0,
    }


def run_benchmark():
    # results[problem_type][digits] = list of dicts
    results = {1: {}, 2: {}}
    global bf_timed_out, ph_timed_out

    for ptype in (1, 2):
        print(f"\n{'='*60}")
        print(f"Problem type {ptype}")
        print(f"{'='*60}")

        bf_timed_out = False
        ph_timed_out = False
        
        digits = 3

        while not (bf_timed_out and ph_timed_out):
            if digits >= 20:
                print(f"[ALARM] EMERGENCY STOP WE ALREADY REACHED {digits} digits")
                break
            print(f"  digits={digits}  ", end="", flush=True)
            runs = []

            for run in range(RUNS_PER_CASE):
                r = generate_and_solve(ptype, digits)
                if r is None:
                    print("!", end="", flush=True)
                    continue
                runs.append(r)
                print(".", end="", flush=True)

            print()

            if not runs:
                results[ptype][digits] = None
                digits += 1
                continue

            valid_bf = [r["ms_bf"] for r in runs if r["ms_bf"] != float("inf")]
            avg_bf = sum(valid_bf) / len(valid_bf) if valid_bf else float("inf")
            valid_ph = [r["ms_ph"] for r in runs if r["ms_ph"] != float("inf")]
            avg_ph = sum(valid_ph) / len(valid_ph) if valid_ph else float("inf")
            results[ptype][digits] = {"avg_bf": avg_bf, "avg_ph": avg_ph, "n": len(runs)}
            digits += 1

    # підсумкова таблиця
    print(f"\n{'='*60}")
    print("Summary (avg ms over runs)")
    print(f"{'='*60}")

    header = f"{'type':>6}  {'digits':>6}  {'brute_force_ms':>16}  {'pohlig_hellman_ms':>18}  {'runs':>5}"
    print(header)
    print("-" * len(header))

    for ptype in (1, 2):
        for digits in sorted(results[ptype].keys()):
            r = results[ptype][digits]
            if r is None:
                print(f"{ptype:>6}  {digits:>6}  {'N/A':>16}  {'N/A':>18}  {'0':>5}")
            else:
                bf_str = "timeout" if r["avg_bf"] == float("inf") else f'{r["avg_bf"]:>16.4f}'
                print(f"{ptype:>6}  {digits:>6}  {bf_str:>16}  {r['avg_ph']:>18.4f}  {r['n']:>5}")


if __name__ == "__main__":
    run_benchmark()
