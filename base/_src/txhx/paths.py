# paths.py
import os
from datetime import datetime
from typing import Optional, Tuple

from config import SOURCE_PATH, BACKUP_ROOT, FILES_TO_PUSH, FOLDER_TO_PUSH


def normalize_path(path: str) -> str:
    return os.path.normpath(path)


def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def get_backup_folder_name() -> str:
    now = datetime.now()
    return now.strftime("%Y-%m-%d_%H-%M")


def get_backup_path() -> str:
    folder_name = get_backup_folder_name()
    return os.path.join(BACKUP_ROOT, folder_name)


def check_source_available() -> Tuple[bool, Optional[str]]:
    """Проверяет доступность сетевого пути."""
    if not os.path.exists(SOURCE_PATH):
        return False, f"Сетевой путь недоступен: {SOURCE_PATH}"
    if not os.access(SOURCE_PATH, os.R_OK):
        return False, f"Нет прав на чтение: {SOURCE_PATH}"
    return True, None


def list_source_files() -> list[str]:
    """Список файлов в SOURCE_PATH (только файлы)."""
    files = []
    try:
        for name in os.listdir(SOURCE_PATH):
            full = os.path.join(SOURCE_PATH, name)
            if os.path.isfile(full):
                files.append(full)
    except Exception:
        pass
    return files


def build_push_items(project_root: str) -> list[dict]:
    """Формирует список элементов для «push» в backup: файлы и папка XML."""
    items = []

    # файлы list-points.*
    for fname in FILES_TO_PUSH:
        src = os.path.join(project_root, fname)
        if os.path.isfile(src):
            items.append({
                "type": "file",
                "src": src,
                "dst_name": fname,
            })

    # папка XML
    xml_src = os.path.join(project_root, FOLDER_TO_PUSH)
    if os.path.isdir(xml_src):
        items.append({
            "type": "folder",
            "src": xml_src,
            "dst_name": FOLDER_TO_PUSH,
        })

    return items
