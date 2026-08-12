# report.py
import os
from datetime import datetime

REPORT_FILE = "backup/report.html"

def init_report():
    """
    Создаёт каркас ТОЛЬКО если файла нет.
    Если файл уже есть — ничего не делает, чтобы не ломать накопленную историю.
    """
    os.makedirs("backup", exist_ok=True)

    if os.path.exists(REPORT_FILE):
        # Файл уже есть: значит, история уже началась. Не перезаписываем.
        return

    # Создаём каркас один раз
    html = """<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>TXHX Backup Report (History)</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ccc; padding: 6px 8px; text-align: left; }
        th { background-color: #f0f0f0; }
        .status-ok { color: green; font-weight: bold; }
        .status-fail { color: red; font-weight: bold; }
        .status-partial { color: orange; font-weight: bold; }
        .session-separator { font-weight: bold; font-size: 1.1em; color: #333; }
    </style>
</head>
<body>
    <h1>TXHX Backup Report — History</h1>
    <table>
        <thead>
            <tr>
                <th>Time</th>
                <th>Action</th>
                <th>Details</th>
                <th>Status</th>
            </tr>
        </thead>
        <tbody id="report-body">
"""
    with open(REPORT_FILE, "w", encoding="utf-8") as f:
        f.write(html)


def append_report_row(action: str, details: str, status: str):
    """Дописывает одну строку в конец таблицы (режим 'a')."""
    time_str = datetime.now().strftime("%Y-%m-%d %H:%M")

    status_class = ""
    if status == "OK":
        status_class = "status-ok"
    elif status == "FAIL":
        status_class = "status-fail"
    elif status == "PARTIAL":
        status_class = "status-partial"

    row = f"""            <tr>
                <td>{time_str}</td>
                <td>{action}</td>
                <td>{details}</td>
                <td class="{status_class}">{status}</td>
            </tr>
"""
    with open(REPORT_FILE, "a", encoding="utf-8") as f:
        f.write(row)


def finalize_report():
    """
    Добавляет разделитель сессии.
    Таблицу НЕ закрываем, чтобы можно было дальше накапливать строки.
    Автопрокрутку тоже не делаем: она будет всегда мотать к самому концу,
    и ты не увидишь старые сессии.
    """
    separator = """            <tr>
                <td colspan="4" class="session-separator">--- Session ended ---</td>
            </tr>
"""
    with open(REPORT_FILE, "a", encoding="utf-8") as f:
        f.write(separator)
