#!/usr/bin/env python3
#! Generado con IA: ChatGPT-6 Modelo Astra; Prompt con enfoque de detección de casos bordes.

"""Prueba find en una copia temporal sin modificar fs.img del repositorio."""

import os
from pathlib import Path
import re
import select
import shutil
import signal
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def wait_for(proc, marker, timeout, show_progress=False):
    output = bytearray()
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        ready, _, _ = select.select([proc.stdout], [], [], 0.2)
        if ready:
            chunk = os.read(proc.stdout.fileno(), 4096)
            if not chunk:
                break
            output.extend(chunk)
            if show_progress:
                print(chunk.decode(errors="replace"), end="", flush=True)
            if marker in output:
                return output.decode(errors="replace")
        if proc.poll() is not None:
            break
    raise RuntimeError(f"No se recibio {marker!r}:\n{output.decode(errors='replace')}")


def main():
    with tempfile.TemporaryDirectory(prefix="xv6-find-") as folder:
        work = Path(folder)
        # Leer el contenido actual de los archivos versionados, no el de HEAD.
        names = subprocess.check_output(
            ["git", "ls-files", "-z"], cwd=ROOT
        ).decode().split("\0")
        for name in filter(None, names):
            source = ROOT / name
            if source.is_file():
                target = work / name
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)
        shutil.copy2(ROOT / "tests/find_cases.c", work / "user/findtest.c")
        makefile = work / "Makefile"
        # El auxiliar y la medicion de pila solo se agregan a la copia temporal.
        makefile.write_text(
            makefile.read_text().replace(
                "\nfs.img:", "\nUPROGS += $U/_findtest\n\nfs.img:", 1
            ) + "\nCFLAGS += -fstack-usage\n"
        )
        build = subprocess.run(
            ["make", "-j2", "check-qemu-version", "kernel/kernel", "fs.img"],
            cwd=work, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, timeout=120,
        )
        if build.returncode:
            raise RuntimeError("Error de compilacion:\n" + build.stdout)
        print("Compilacion desde cero: OK", flush=True)
        frames = {}
        for source in ("find", "printf", "ulib"):
            for line in (work / f"user/{source}.su").read_text().splitlines():
                location, size, kind = line.split("\t")
                if kind != "static":
                    raise RuntimeError("Uso de pila no estatico: " + line)
                frames[location.rsplit(":", 1)[1]] = int(size)
        # main, ocho search, marcos de biblioteca (sumados conservadoramente)
        # y 512 bytes para argumentos normales y alineacion del stack inicial.
        bound = frames["main"] + 8 * frames["search"] + 512
        bound += sum(size for name, size in frames.items()
                    if name not in ("main", "search", "valid_path"))
        if bound >= 4096:
            raise RuntimeError(f"Sin margen conservador de pila: {bound} bytes")
        print(f'Pila por search: {frames["search"]} bytes', flush=True)
        print(f"Cota conservadora: {bound} / 4096 bytes", flush=True)

        proc = subprocess.Popen(
            ["make", "qemu"], cwd=work, stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            start_new_session=True,
        )
        try:
            boot = wait_for(proc, b"$ ", 20)
            if "panic" in boot:
                raise RuntimeError(boot)
            proc.stdin.write(b"findtest\n")
            proc.stdin.flush()
            result = wait_for(proc, b"$ ", 180, show_progress=True)
            if "FAIL:" in result or "panic" in result or not re.search(
                r"FIND TESTS PASSED: \d+", result
            ):
                raise RuntimeError("Las pruebas de find no terminaron correctamente")
        finally:
            if proc.poll() is None:
                try:
                    proc.stdin.write(b"\x01x")
                    proc.stdin.flush()
                    proc.wait(timeout=5)
                except (BrokenPipeError, subprocess.TimeoutExpired):
                    os.killpg(proc.pid, signal.SIGTERM)
                    proc.wait(timeout=5)
            proc.stdin.close()
            proc.stdout.close()


if __name__ == "__main__":
    main()
