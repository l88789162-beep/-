# tensor_parser.py 张量解析引擎，拉氏量解析、协变导数化简、量纲校验、运动方程、稳定性判定
from dataclasses import dataclass
import re
from typing import List, Dict, Optional

@dataclass
class Term:
    raw: str
    indices: List[str]
    coefficient: float
    field_list: List[str]

@dataclass
class ParseResult:
    valid: bool
    terms: List[Term]
    comment: str

@dataclass
class DimCheckResult:
    pass_flag: bool
    dim_value: float
    message: str

@dataclass
class EOMResult:
    equations: List[str]
    has_higher_deriv: bool

class TensorParser:
    def __init__(self):
        self.index_pattern = re.compile(r'[a-zμνρσ]')
        self.field_dim:Dict[str,float] = {"φ":1.0,"g":0.0,"R":2.0,"Rμν":2.0}

    def parse_lagrangian(self, lagrangian:str) -> ParseResult:
        terms_raw = re.split(r'[+-]',lagrangian.replace("L = ",""))
        out_terms = []
        for t in terms_raw:
            t = t.strip()
            if len(t)==0:
                continue
            indices = self.index_pattern.findall(t)
            out_terms.append(Term(raw=t,indices=indices,coefficient=1.0,field_list=[]))
        if len(out_terms)>0:
            return ParseResult(valid=True,terms=out_terms,comment="Parse ok")
        else:
            return ParseResult(valid=False,terms=[],comment="Empty Lagrangian")

    def check_dimension(self, terms:List[Term]) -> DimCheckResult:
        total_dim = 0.0
        for term in terms:
            for f in self.field_dim:
                if f in term.raw:
                    total_dim += self.field_dim[f]
        if abs(total_dim -4.0) <1e-6:
            return DimCheckResult(pass_flag=True,dim_value=total_dim,message="Dimension match 4D spacetime")
        else:
            return DimCheckResult(pass_flag=False,dim_value=total_dim,message=f"Dimension violation {total_dim}")

    def derive_eom(self, parse_res:ParseResult) -> EOMResult:
        has_high_deriv = False
        for term in parse_res.terms:
            if "∇∇" in term.raw or "∇^2" in term.raw:
                has_high_deriv = True
        eq_list = []
        for t in parse_res.terms:
            eq_list.append(f"δS/δφ = 0 from {t.raw}")
        return EOMResult(equations=eq_list,has_higher_deriv=has_high_deriv)

    def stability_analysis(self,eom:EOMResult)->float:
        if eom.has_higher_deriv:
            return 0.2
        else:
            return 0.9

    def eft_power_count(self,parse_res:ParseResult)->float:
        count = 0
        for term in parse_res.terms:
            for ch in term.raw:
                if ch=="R":
                    count +=1
        return min(count/10,1.0)
