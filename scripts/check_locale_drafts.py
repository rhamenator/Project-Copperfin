#!/usr/bin/env python3
"""Validate first-pass locale drafts and their exact independent review records."""

from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import sys

sys.dont_write_bytecode = True
from check_locale_review_status import protected_syntax  # noqa: E402


ROOT = Path(__file__).resolve().parent.parent
DRAFT_ROOT = ROOT / "docs" / "localization-drafts"
PLACEHOLDER = re.compile(r"\{[^{}]+\}")
BATCH_NAME = re.compile(r"(?P<locale>[a-z]{2,3}(?:-[A-Za-z0-9]+)+)-(?P<index>\d{3})\.json")


def digest(value: str) -> str:
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def validate_directory(path: Path, source: dict[str, str]) -> None:
    source_keys = list(source)
    drafts: dict[tuple[str, str], str] = {}
    batch_numbers: dict[str, set[int]] = {}
    for batch_path in sorted(path.glob("*.json")):
        match = BATCH_NAME.fullmatch(batch_path.name)
        if not match:
            continue
        locale, number = match.group("locale"), int(match.group("index"))
        batch = json.loads(batch_path.read_text(encoding="utf-8"))
        if batch.get("locale") != locale:
            raise ValueError(f"Locale mismatch: {batch_path}")
        expected_keys = source_keys[number * 40:(number + 1) * 40]
        expected_input = [(key, source[key]) for key in expected_keys]
        actual_input = [(row["key"], row["source"]) for row in batch["input"]]
        entries = batch["entries"]
        if actual_input != expected_input or [row["key"] for row in entries] != expected_keys:
            raise ValueError(f"Source order or text mismatch: {batch_path}")
        if number in batch_numbers.setdefault(locale, set()):
            raise ValueError(f"Duplicate batch: {batch_path}")
        batch_numbers[locale].add(number)
        for row in entries:
            key, translation = row["key"], row["translation"]
            if not isinstance(translation, str) or not translation.strip():
                raise ValueError(f"Blank translation: {batch_path}:{key}")
            if Counter(PLACEHOLDER.findall(translation)) != Counter(PLACEHOLDER.findall(source[key])):
                raise ValueError(f"Placeholder drift: {batch_path}:{key}")
            if translation.count("\n") != source[key].count("\n") or \
                    len(translation) - len(translation.rstrip(" \t\n")) != \
                    len(source[key]) - len(source[key].rstrip(" \t\n")):
                raise ValueError(f"Layout whitespace drift: {batch_path}:{key}")
            pair = (locale, key)
            if pair in drafts:
                raise ValueError(f"Duplicate draft key: {batch_path}:{key}")
            drafts[pair] = translation

    verdicts: dict[tuple[str, str], str] = {}
    for review_path in sorted(path.glob("copperfin-*review*.json")):
        review = json.loads(review_path.read_text(encoding="utf-8"))
        entries = review["entries"]
        for row in entries:
            locale = row.get("locale") or review.get("locale")
            if not locale:
                raise ValueError(f"Review locale missing: {review_path}:{row['key']}")
            key = row["key"]
            pair = (locale, key)
            if pair not in drafts or pair in verdicts:
                raise ValueError(f"Missing draft or duplicate verdict: {review_path}:{key}")
            if row["sourceSha256"] != digest(source[key]) or \
                    row["translationSha256"] != digest(drafts[pair]):
                raise ValueError(f"Stale review hash: {review_path}:{key}")
            verdict = row["verdict"]
            if verdict not in ("accept", "revise"):
                raise ValueError(f"Invalid verdict: {review_path}:{key}")
            if verdict == "accept" and protected_syntax(source[key]) != protected_syntax(drafts[pair]):
                raise ValueError(f"Accepted protected-syntax drift: {review_path}:{key}")
            verdicts[pair] = verdict

    if drafts.keys() != verdicts.keys():
        missing = sorted(drafts.keys() - verdicts.keys())
        raise ValueError(f"Drafts without independent verdicts: {missing[:5]}")
    for locale, numbers in sorted(batch_numbers.items()):
        if numbers != set(range(max(numbers) + 1)):
            raise ValueError(f"Noncontiguous batches: {locale}")
        counts = Counter(verdicts[pair] for pair in drafts if pair[0] == locale)
        print(f"{locale}: {sum(counts.values())} drafted; "
              f"{counts['accept']} accepted; {counts['revise']} need revision")


def main() -> None:
    source = json.loads((ROOT / "resources/locales/en-US/strings.json").read_text(encoding="utf-8"))
    for path in sorted(DRAFT_ROOT.iterdir()):
        if path.is_dir() and any(path.glob("*.json")):
            validate_directory(path, source)


if __name__ == "__main__":
    main()
