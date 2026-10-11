#!/usr/bin/env python3
"""Capture real UART output while running one already-built assignment app.

No reconstructed screenshots. Saved byte stream, timestamped events, tool log,
and artifact hashes preserve the scope and identity of each native board run.
"""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import selectors
import subprocess
import termios
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exercise', choices=('3', '4', '5', '7'))
    parser.add_argument('--serial', default='/dev/serial/by-id/usb-Digilent_Digilent_Adept_USB_Device_210279655163-if01-port0')
    parser.add_argument('--cable', default='210279655163A')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--xsdb', default='/home/firas/AMD/2026.1/Vitis/bin/xsdb')
    args = parser.parse_args()
    here = Path(__file__).resolve().parent
    app = {'3': 'ex3_console', '4': 'ex4_matrix', '5': 'ex5_matrix', '7': 'neuralNetTest'}[args.exercise]
    ws = here / ('Ex7_vitis' if args.exercise == '7' else 'Ex345_vitis')
    bit = here / ('Ex7/Ex7.runs/impl_1/system_wrapper.bit' if args.exercise == '7'
                  else 'generated_ex345/Ex345/Ex345.runs/impl_1/system_wrapper.bit')
    elf = ws / app / 'build' / (app + '.elf')
    metadata = {'exercise': args.exercise, 'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
                'cable': args.cable, 'serial': args.serial,
                'bitstream': str(bit), 'elf': str(elf)}
    for label, artifact in [('bitstream_sha256', bit), ('elf_sha256', elf)]:
        with artifact.open('rb') as stream:
            metadata[label] = hashlib.file_digest(stream, 'sha256').hexdigest()
    args.output.mkdir(parents=True, exist_ok=False)
    fd = os.open(args.serial, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
    old = termios.tcgetattr(fd)
    cfg = termios.tcgetattr(fd)
    cfg[0], cfg[1], cfg[2], cfg[3] = 0, 0, termios.CS8 | termios.CREAD | termios.CLOCAL, 0
    cfg[4] = cfg[5] = termios.B115200
    cfg[6][termios.VMIN] = cfg[6][termios.VTIME] = 0
    termios.tcsetattr(fd, termios.TCSANOW, cfg)
    termios.tcflush(fd, termios.TCIFLUSH)
    commands = {'3': ['1', '2', 'mult'], '4': ['mult'], '5': ['mult'] * 3, '7': []}[args.exercise]
    sent = 0
    data = bytearray()
    pending_line = b''
    counters = []
    started = time.monotonic()
    proc = None
    success = False
    print('NATIVE_EVIDENCE=' + str(args.output), flush=True)
    try:
        with (args.output / 'xsdb.log').open('xb') as debug, \
             (args.output / 'uart.log').open('xb') as uart, \
             (args.output / 'events.jsonl').open('x') as events:
            proc = subprocess.Popen([args.xsdb, '-no-ini', str(here / 'run_assignment_on_board.tcl'),
                                     args.exercise, args.cable], stdout=debug, stderr=subprocess.STDOUT)
            selector = selectors.DefaultSelector()
            selector.register(fd, selectors.EVENT_READ)
            quiet_since = time.monotonic()
            while time.monotonic() - started < 90:
                for key, _ in selector.select(timeout=0.2):
                    chunk = os.read(key.fd, 16384)
                    if not chunk:
                        continue
                    now = time.monotonic() - started
                    quiet_since = time.monotonic()
                    data.extend(chunk)
                    uart.write(chunk)
                    uart.flush()
                    print(chunk.decode(errors='replace'), end='', flush=True)
                    events.write(json.dumps({'seconds': now, 'uart': chunk.decode(errors='replace')}) + '\n')
                    pending_line += chunk
                    while b'\n' in pending_line:
                        line, pending_line = pending_line.split(b'\n', 1)
                        match = re.search(rb'Counter: (\d+)', line)
                        if match:
                            counters.append({'value': int(match[1]), 'host_receive_seconds': now})
                prompt_count = data.count(b'CMD:> ')
                if sent < len(commands) and prompt_count > sent:
                    command = commands[sent]
                    os.write(fd, (command + '\r').encode())
                    events.write(json.dumps({'seconds': time.monotonic()-started, 'sent': command}) + '\n')
                    events.flush()
                    sent += 1
                complete = (b'Overall verification: PASS' in data if args.exercise == '7'
                            else sent == len(commands) and prompt_count > sent)
                if proc.poll() is not None and (proc.returncode != 0 or
                                               (complete and time.monotonic()-quiet_since > 1)):
                    break
            selector.close()
            if proc.poll() is None:
                proc.terminate()
                try:
                    proc.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    proc.kill()
                    proc.wait()
            success = proc.returncode == 0 and complete
            if args.exercise == '3':
                success = success and [x['value'] for x in counters] == list(range(16))
                intervals = [b['host_receive_seconds'] - a['host_receive_seconds']
                             for a, b in zip(counters, counters[1:])]
                interval_ok = len(intervals) == 15 and all(
                    0.75 <= interval <= 1.25 for interval in intervals)
                metadata['counter_uart_intervals_seconds'] = intervals
                metadata['counter_uart_interval_check'] = 'PASS' if interval_ok else 'FAIL'
                # Host UART timestamps include USB scheduling jitter. This is
                # an approximate cadence check, not optical LED measurement.
                success = success and interval_ok
            if args.exercise == '5':
                success = success and data.count(b'Verification: PASS') == len(commands)
            if args.exercise in ('3', '4', '5'):
                # Check the independently known literal-transpose fixture, not
                # just a software/hardware equality message.
                products = re.findall(
                    rb'P(?: \((?:software|hardware)\))? =\r?\n'
                    rb'((?:[ \t]*\d+(?:[ \t]+\d+){3}[ \t]*\r?\n){4})', data)
                expected = [value for value in (30, 70, 110, 150) for _ in range(4)]
                required_products = 6 if args.exercise == '5' else 1
                matrix_ok = len(products) == required_products and all(
                    list(map(int, re.findall(rb'\d+', product))) == expected
                    for product in products)
                success = success and matrix_ok
                metadata['known_matrix_fixture'] = 'PASS' if matrix_ok else 'FAIL'
            if b'FAIL' in data or b'failed' in data or b'timed out' in data:
                success = False
            metadata.update(xsdb_exit=proc.returncode, command_count=sent, counters=counters,
                            capture_check='PASS' if success else 'FAIL',
                            physical_led_observation='not observed by capture script')
    finally:
        termios.tcsetattr(fd, termios.TCSANOW, old)
        os.close(fd)
        (args.output / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n')
    print('\nNATIVE_CAPTURE_CHECK=' + ('PASS' if success else 'FAIL'), flush=True)
    return 0 if success else 1


if __name__ == '__main__':
    raise SystemExit(main())
