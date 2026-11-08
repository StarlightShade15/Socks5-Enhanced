import os
import sys
from urllib.parse import quote

try:
	import requests
	import socks  # noqa: F401 - requests uses PySocks for SOCKS proxies
except ImportError:
	raise SystemExit(
		"SOCKS support is missing; create a virtual environment and install it with: "
		"python3 -m venv .venv && .venv/bin/python -m pip install 'requests[socks]'"
	)


PROXY_USERNAME = os.environ.get("SOCKS5_USERNAME", "user")
PROXY_PASSWORD = os.environ.get(
	"SOCKS5_PASSWORD", "4uSmFzdYzggJhj90QcfB3YphqYH2I3qs"
)
PROXY = (
	f"socks5h://{quote(PROXY_USERNAME, safe='')}:"
	f"{quote(PROXY_PASSWORD, safe='')}@127.0.0.1:6969"
)
URL = "https://www.google.com/"

try:
	response = requests.get(
		URL,
		proxies={"http": PROXY, "https": PROXY},
		timeout=(5, 15),
	)
	response.raise_for_status()
except requests.exceptions.RequestException as error:
	print(f"SOCKS5 request failed: {error}", file=sys.stderr)
	raise SystemExit(1)

print(f"SOCKS5 request succeeded: HTTP {response.status_code} from {response.url}")