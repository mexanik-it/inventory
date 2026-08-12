# report.py
import os
from datetime import datetime
from typing import List, Dict, Any

from config import BACKUP_ROOT, REPORT_FILE


REPORT_PATH = os.path.join(BACKUP_ROOT, REPORT_FILE)


def init_report() -> None:
    ensure_dir(BACKUP_ROOT)
    if not os.path.exists(REPORT_PATH):
        with open(REPORT_PATH, "w", encoding="utf-8") as f:
            f.write("""<!DOCTYPE html>
<html><head><meta charset="utf-8"><title>TXHX Backup Report</title></head>
<body><h1>TXHX Backup Report</h1><table border="1"><thead>
<tr><th>Time</th><th>Action</th><th>Details</th><th>Status</th></tr>
</thead><tbody>
""")


def append_report_row(action: str, details: str, status: str) -> None:
    """Дозаписывает одну строку в report.html."""
    time_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    # экранирование для HTML
    details_esc = (
        str(details)
        .replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
    )
    row = (
        f"<tr>"
        f"<td>{time_str}</td>"
        f"<td>{action}</td>"
        f"<td>{details_esc}</td>"
        f"<td>{status}</td>"
        f"</tr>\n"
    )
    with open(REPORT_PATH, "a", encoding="utf-8") as f:
        f.write(row)


def finalize_report() -> None:
    with open(REPORT_PATH, "a", encoding="utf-8") as f:
        f.write("</tbody></table></body></html>\n")


def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)
