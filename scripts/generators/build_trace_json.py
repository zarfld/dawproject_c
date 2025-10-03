#!/usr/bin/env python3
"""Build Traceability JSON

Consumes build/spec-index.json and produces build/traceability.json capturing
forward and backward links for visualization or further QA tooling.
"""
from __future__ import annotations
import json
from pathlib import Path
from collections import defaultdict

ROOT = Path(__file__).resolve().parents[2]
INDEX = ROOT / 'build' / 'spec-index.json'
OUT = ROOT / 'build' / 'traceability.json'

PREFIX_ORDER = ['StR', 'REQ', 'ARC', 'ADR', 'QA', 'TEST']


def main() -> int:
    if not INDEX.exists():
        print('Missing spec-index.json; run spec_parser first')
        return 1
    data = json.loads(INDEX.read_text(encoding='utf-8'))
    items = data.get('items', [])
    by_id = {i['id']: i for i in items}

    forward = {i['id']: i.get('references', []) for i in items}
    backward = defaultdict(list)
    for src, refs in forward.items():
        for r in refs:
            backward[r].append(src)

    # Simple completeness metrics
    metrics = {}
    for prefix in PREFIX_ORDER:
        ids = [i['id'] for i in items if i['id'].startswith(prefix)]
        linked = [i for i in ids if any(b for b in forward[i])]
        metrics[prefix.lower() if prefix != 'REQ' else 'requirement'] = {
            'total': len(ids),
            'with_links': len(linked),
            'coverage_pct': (len(linked) / len(ids) * 100) if ids else 100.0,
        }

    # Requirement-specific linkage dimensions
    req_ids = [i['id'] for i in items if i['id'].startswith('REQ')]
    def req_link_stat(target_prefix: str):
        count_total = len(req_ids)
        count_with = 0
        for rid in req_ids:
            refs = forward.get(rid, [])
            if any(r.startswith(target_prefix) for r in refs):
                count_with += 1
        pct = (count_with / count_total * 100) if count_total else 100.0
        return {'total_requirements': count_total, 'requirements_with_link': count_with, 'coverage_pct': pct}

    metrics['requirement_to_ADR'] = req_link_stat('ADR')
    metrics['requirement_to_scenario'] = req_link_stat('QA')
    metrics['requirement_to_test'] = req_link_stat('TEST')

    OUT.write_text(
        json.dumps(
            {
                'items': items,
                'forward': forward,
                'backward': backward,
                'metrics': metrics,
            },
            indent=2,
        ),
        encoding='utf-8',
    )
    print(f"Wrote {OUT}")
    return 0

if __name__ == '__main__':
    import sys
    raise SystemExit(main())
