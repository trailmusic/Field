#!/usr/bin/env python3
import sys, re, json
from pathlib import Path
import yaml

# --- config
ROOT = Path(__file__).resolve().parents[2]
YAML_REG = ROOT / "docs/params/ParamRegistry.yaml"
ROOTS = [ROOT/"Source/core", ROOT/"Source/modules", ROOT/"Source/features", ROOT/"Source/processor"]

def main():
    if not YAML_REG.exists():
        print(json.dumps({"error": f"Registry not found: {YAML_REG}"}, indent=2))
        sys.exit(1)

    # --- load YAML ids
    doc = yaml.safe_load(YAML_REG.read_text())
    yaml_ids = set()
    for p in doc.get("params", []):
        _id = p.get("id")
        if _id:
            # Treat templated IDs (e.g. dyneq.b[i].freq.hz) as patterns; skip from strict check
            if "[" in _id or "]" in _id:
                continue
            yaml_ids.add(_id)

    # --- scrape code string-literals that look like param IDs
    code_ids = set()
    STRING = re.compile(r'"([a-zA-Z0-9_.:-]+)"')
    PARAMISH = re.compile(r"[a-z]+(\.[a-z0-9]+){1,}")

    def walk(root: Path):
        for p in root.rglob("*"):
            if p.suffix in (".h", ".hpp", ".cpp", ".mm", ".cxx", ".cc"):
                yield p

    for root in ROOTS:
        for p in walk(root):
            try:
                s = p.read_text(errors="ignore")
            except Exception:
                continue
            for m in STRING.finditer(s):
                t = m.group(1)
                if PARAMISH.fullmatch(t):
                    code_ids.add(t)

    missing_in_code = sorted(yaml_ids - code_ids)
    missing_in_yaml = sorted(code_ids - yaml_ids)

    print(json.dumps({
        "yaml_total": len(yaml_ids),
        "code_total": len(code_ids),
        "missing_in_code": missing_in_code,
        "missing_in_yaml": missing_in_yaml
    }, indent=2))

    if missing_in_code or missing_in_yaml:
        print("\nPARAM REGISTRY DRIFT DETECTED.", file=sys.stderr)
        if missing_in_code:
            print(f"  IDs present in YAML but not found in code: {len(missing_in_code)}", file=sys.stderr)
        if missing_in_yaml:
            print(f"  IDs found in code but not present in YAML: {len(missing_in_yaml)}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()


