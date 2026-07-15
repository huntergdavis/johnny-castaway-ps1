#!/usr/bin/env python3
"""Serve the Johnny Castaway PS1 site plus the local development BIN/CUE.

Mirrors the Zoomies browser prototype: the public site (docs/) is served
as-is, and the private development disc is exposed under /game/ with a
dynamic metadata.json, so nothing disc- or BIOS-shaped ever enters the
repository. Rebuild the disc, reload the page, and the new image appears
under a new build identifier.
"""

from __future__ import annotations

import argparse
import email.utils
import functools
import json
import os
import socket
from datetime import datetime
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SITE_ROOT = PROJECT_ROOT / "docs"
DEFAULT_DISC = PROJECT_ROOT / "johnnycastawayps1.bin"
DISC_NAME = "johnnycastawayps1"


class JCDevHandler(SimpleHTTPRequestHandler):
    server_version = "JCDevServer/0.1"

    @property
    def disc_path(self) -> Path:
        return self.server.disc_path  # type: ignore[attr-defined]

    @property
    def cue_path(self) -> Path:
        return self.server.cue_path  # type: ignore[attr-defined]

    def _request_path(self) -> str:
        return urlsplit(self.path).path

    def _build_id(self) -> str:
        stat = self.disc_path.stat()
        return f"{stat.st_mtime_ns:x}-{stat.st_size:x}"

    def _metadata(self) -> bytes:
        stat = self.disc_path.stat()
        build_id = self._build_id()
        modified = datetime.fromtimestamp(stat.st_mtime).astimezone()
        payload = {
            "bin_size": stat.st_size,
            "bin_url": f"/game/{DISC_NAME}.bin?v={build_id}",
            "bios": "HLE",
            "build_id": build_id,
            "build_label": modified.strftime("build %Y-%m-%d %H:%M:%S"),
            "core": "pcsx_rearmed",
            "cue_url": f"/game/{DISC_NAME}.cue?v={build_id}",
        }
        return json.dumps(payload, separators=(",", ":")).encode("utf-8")

    def _send_bytes(self, data: bytes, content_type: str, head_only: bool) -> None:
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        if not head_only:
            self.wfile.write(data)

    def _send_disc_file(self, path: Path, content_type: str):
        try:
            file_handle = path.open("rb")
        except OSError:
            self.send_error(404, "Development disc is unavailable")
            return None

        stat = os.fstat(file_handle.fileno())
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(stat.st_size))
        self.send_header("Last-Modified", email.utils.formatdate(stat.st_mtime, usegmt=True))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        return file_handle

    def send_head(self):
        path = self._request_path()
        if path == f"/game/{DISC_NAME}.bin":
            return self._send_disc_file(self.disc_path, "application/octet-stream")
        if path == f"/game/{DISC_NAME}.cue":
            return self._send_disc_file(self.cue_path, "text/plain; charset=utf-8")
        return super().send_head()

    def do_GET(self) -> None:
        if self._request_path() == "/game/metadata.json":
            self._send_bytes(self._metadata(), "application/json", head_only=False)
            return
        super().do_GET()

    def do_HEAD(self) -> None:
        if self._request_path() == "/game/metadata.json":
            self._send_bytes(self._metadata(), "application/json", head_only=True)
            return
        super().do_HEAD()

    def end_headers(self) -> None:
        if not any(
            header.lower().startswith(b"cache-control:")
            for header in self._headers_buffer
        ):
            self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        super().end_headers()

    def log_message(self, message: str, *args: object) -> None:
        status = str(args[1]) if len(args) > 1 else ""
        if status.startswith(("4", "5")):
            super().log_message(message, *args)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--disc", type=Path, default=DEFAULT_DISC)
    parser.add_argument("--site", type=Path, default=SITE_ROOT)
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8080)
    return parser.parse_args()


def lan_address() -> str | None:
    probe = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        probe.connect(("192.0.2.1", 9))
        return probe.getsockname()[0]
    except OSError:
        return None
    finally:
        probe.close()


def open_server(host: str, first_port: int, handler):
    ports = [0] if first_port == 0 else range(first_port, first_port + 20)
    last_error: OSError | None = None
    for port in ports:
        try:
            return ThreadingHTTPServer((host, port), handler)
        except OSError as error:
            last_error = error
    raise RuntimeError(
        f"no available port from {first_port} through {first_port + 19}"
    ) from last_error


def main() -> None:
    args = parse_args()
    site = args.site.expanduser().resolve()
    disc = args.disc.expanduser().resolve()
    cue = disc.with_suffix(".cue")

    if not site.joinpath("play", "online", "index.html").is_file():
        raise SystemExit(f"ERROR: browser player not found under {site}")
    if not disc.is_file():
        raise SystemExit(f"ERROR: development BIN not found: {disc}")
    if not cue.is_file():
        raise SystemExit(f"ERROR: development CUE not found: {cue}")
    if f'FILE "{DISC_NAME}.bin" BINARY' not in cue.read_text(encoding="ascii"):
        raise SystemExit(f"ERROR: CUE does not reference {DISC_NAME}.bin: {cue}")

    handler = functools.partial(JCDevHandler, directory=str(site))
    server = open_server(args.host, args.port, handler)
    server.disc_path = disc  # type: ignore[attr-defined]
    server.cue_path = cue  # type: ignore[attr-defined]
    server.daemon_threads = True

    port = server.server_address[1]
    address = lan_address()
    print("Johnny Castaway browser prototype: PCSX-ReARMed with HLE BIOS", flush=True)
    print(f"Disc: {cue}", flush=True)
    print(f"Local: http://127.0.0.1:{port}/play/online/", flush=True)
    if address:
        print(f"LAN:   http://{address}:{port}/play/online/", flush=True)
    print("Press Ctrl-C to stop.", flush=True)

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
