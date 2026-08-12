# backup_engine.py
import os
import shutil
from typing import Tuple

from paths import get_backup_path, build_push_items
from report import append_report_row
from config import SOURCE_PATH


def ensure_backup_dir(backup_dir: str) -> None:
    """Гарантированно создаёт папку для бэкапа, включая промежуточные уровни."""
    try:
        os.makedirs(backup_dir, exist_ok=True)
        append_report_row("MKDIR", f"Created backup dir: {backup_dir}", "OK")
    except Exception as e:
        msg = f"Cannot create backup dir '{backup_dir}': {e}"
        print(f"ERROR: {msg}")
        append_report_row("MKDIR", msg, "FAIL")
        raise


def copy_file(src: str, dst_dir: str, dst_name: str) -> bool:
    dst = os.path.join(dst_dir, dst_name)
    try:
        shutil.copy2(src, dst)
        append_report_row("COPY", f"{src} -> {dst}", "OK")
        return True
    except FileNotFoundError as e:
        # Это как раз твоя ошибка: целевая папка не существует
        msg = f"{src} -> {dst} | Error: Target directory missing: {e}"
        print(f"ERROR: {msg}")
        append_report_row("COPY", msg, "FAIL")
        return False
    except Exception as e:
        msg = f"{src} -> {dst} | Error: {e}"
        print(f"ERROR: {msg}")
        append_report_row("COPY", msg, "FAIL")
        return False


def copy_folder(src: str, dst_dir: str, dst_name: str) -> bool:
    dst = os.path.join(dst_dir, dst_name)
    try:
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(src, dst, dirs_exist_ok=True)
        append_report_row("COPY_FOLDER", f"{src} -> {dst}", "OK")
        return True
    except Exception as e:
        msg = f"{src} -> {dst} | Error: {e}"
        print(f"ERROR: {msg}")
        append_report_row("COPY_FOLDER", msg, "FAIL")
        return False


def backup_source_files(backup_dir: str) -> Tuple[int, int]:
    ok = 0
    fail = 0

    if not os.path.isdir(SOURCE_PATH):
        msg = f"Source path does not exist: {SOURCE_PATH}"
        print(f"ERROR: {msg}")
        append_report_row("LIST_SOURCE", SOURCE_PATH, f"FAIL: {msg}")
        return ok, fail

    try:
        entries = os.listdir(SOURCE_PATH)
    except PermissionError:
        msg = "Permission denied accessing source path"
        print(f"ERROR: {msg}")
        append_report_row("LIST_SOURCE", SOURCE_PATH, f"FAIL: {msg}")
        return ok, fail
    except Exception as e:
        msg = f"Cannot list source: {e}"
        print(f"ERROR: {msg}")
        append_report_row("LIST_SOURCE", SOURCE_PATH, f"FAIL: {msg}")
        return ok, fail

    source_files = [
        name for name in entries
        if os.path.isfile(os.path.join(SOURCE_PATH, name))
    ]

    for name in source_files:
        src = os.path.join(SOURCE_PATH, name)
        dst = os.path.join(backup_dir, name)
        try:
            shutil.copy2(src, dst)
            append_report_row("SOURCE_COPY", f"{name}", "OK")
            ok += 1
        except Exception as e:
            msg = f"{name} | Error: {e}"
            print(f"ERROR: {msg}")
            append_report_row("SOURCE_COPY", msg, "FAIL")
            fail += 1

    return ok, fail


def push_local_items(backup_dir: str, project_root: str) -> Tuple[int, int]:
    ok = 0
    fail = 0
    items = build_push_items(project_root)

    for item in items:
        if item["type"] == "file":
            if copy_file(item["src"], backup_dir, item["dst_name"]):
                ok += 1
            else:
                fail += 1
        elif item["type"] == "folder":
            if copy_folder(item["src"], backup_dir, item["dst_name"]):
                ok += 1
            else:
                fail += 1

    return ok, fail


def perform_backup(project_root: str) -> dict:
    backup_dir = get_backup_path()
    from report import init_report, finalize_report
    init_report()

    append_report_row("START", f"Backup started to {backup_dir}", "RUNNING")

    # ВАЖНО: сначала создаём папку, потом копируем
    ensure_backup_dir(backup_dir)

    src_ok, src_fail = backup_source_files(backup_dir)
    push_ok, push_fail = push_local_items(backup_dir, project_root)

    total_ok = src_ok + push_ok
    total_fail = src_fail + push_fail

    status = "SUCCESS" if total_fail == 0 else "PARTIAL"
    append_report_row(
        "FINISH",
        f"Total OK: {total_ok}, Fail: {total_fail}",
        status,
    )
    finalize_report()

    return {
        "backup_dir": backup_dir,
        "ok": total_ok,
        "fail": total_fail,
        "status": status,
    }
