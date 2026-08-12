# main.py
import argparse
import os
import sys
from datetime import datetime

from txhx import TXHX
from config import BACKUP_ROOT


def print_usage():
    print("TXHX Backup Tool — CLI")
    print("Usage:")
    print("  python main.py backup                  # запустить резервное копирование")
    print("  python main.py restore <folder>       # восстановить из папки backup/<folder>")
    print("  python main.py compress <folder>      # сжать указанную папку backup в ZIP")
    print("  python main.py remove-dups <file>     # удалить дублирующиеся строки в файле")
    print()


def get_backup_folder_list():
    """Возвращает список папок в backup/ с сортировкой по дате (новые первыми)."""
    if not os.path.isdir(BACKUP_ROOT):
        return []
    folders = [
        f for f in os.listdir(BACKUP_ROOT)
        if os.path.isdir(os.path.join(BACKUP_ROOT, f))
    ]
    # формат: YYYY-mm-dd_HH-MM
    def parse_date(name: str):
        try:
            return datetime.strptime(name, "%Y-%m-%d_%H-%M")
        except ValueError:
            return None

    folders_with_date = [(f, parse_date(f)) for f in folders]
    sorted_folders = [
        f for f, d in sorted(
            folders_with_date,
            key=lambda x: x[1] or datetime.min,
            reverse=True
        )
    ]
    return sorted_folders


def main():
    parser = argparse.ArgumentParser(description="TXHX Backup Tool")
    subparsers = parser.add_subparsers(dest="command", help="Доступные команды")

    # backup
    subparsers.add_parser("backup", help="Запустить резервное копирование")

    # restore
    restore_parser = subparsers.add_parser("restore", help="Восстановить из папки backup")
    restore_parser.add_argument("folder", nargs="?", help="Имя папки в backup/ (например, 2026-08-10_10-47)")

    # compress
    compress_parser = subparsers.add_parser("compress", help="Сжать папку backup в ZIP")
    compress_parser.add_argument("folder", nargs="?", help="Имя папки в backup/")

    # remove-dups
    dups_parser = subparsers.add_parser("remove-dups", help="Удалить дублирующиеся строки в файле")
    dups_parser.add_argument("file", help="Путь к текстовому файлу")

    args = parser.parse_args()

    if not args.command:
        print_usage()
        sys.exit(1)

    txhx = TXHX()

    try:
        if args.command == "backup":
            print("Starting backup...")
            result = txhx.backup()
            # Вывод итогов уже есть внутри backup(), но можно продублировать кратко
            print(f"Backup status: {result['status']}")

        elif args.command == "restore":
            if not args.folder:
                folders = get_backup_folder_list()
                if not folders:
                    print("Нет папок в backup/. Запустите сначала backup.")
                    sys.exit(1)
                print("Доступные папки для восстановления:")
                for f in folders[:10]:
                    print(f"  - {f}")
                print("\nУкажите папку явно: python main.py restore <имя_папки>")
                sys.exit(1)

            backup_path = os.path.join(BACKUP_ROOT, args.folder)
            print(f"Restoring from: {backup_path}")
            result = txhx.restore(backup_path)
            print(f"Restore status: {result['status']}")

        elif args.command == "compress":
            if not args.folder:
                folders = get_backup_folder_list()
                if not folders:
                    print("Нет папок в backup/. Запустите сначала backup.")
                    sys.exit(1)
                print("Доступные папки для сжатия:")
                for f in folders[:10]:
                    print(f"  - {f}")
                print("\nУкажите папку явно: python main.py compress <имя_папки>")
                sys.exit(1)

            backup_path = os.path.join(BACKUP_ROOT, args.folder)
            print(f"Compressing: {backup_path}")
            result = txhx.compress(backup_path)
            print(f"Compress status: {result['status']}, archive: {result['archive_path']}")

        elif args.command == "remove-dups":
            file_path = args.file
            if not os.path.isfile(file_path):
                print(f"Файл не найден: {file_path}")
                sys.exit(1)
            print(f"Removing duplicate lines in: {file_path}")
            success = txhx.remove_duplicate_lines(file_path)
            if success:
                print("Duplicate lines removed successfully.")
            else:
                print("Failed to remove duplicate lines.")
                sys.exit(1)

    except RuntimeError as e:
        print(f"Runtime error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
