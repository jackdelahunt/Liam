main_code = """
    fn main() void {

    }
"""

print(main_code)

for i in range(30000):
    print(f"""struct TypeStruct{i} {{
        a: i64,
        b: bool,
        c: f32,
        x: str,
        y: f64,
        z: u32
    }}""")

for i in range(20000):
    print(f"""fn add_{i}(a: i64, b: i64, c: i64, d: bool, e: str) void {{""")
    for x in range(100):
        print(f"""
        let n_1_{x} := {x};
        let s_1_{x} := \"{x}\";
        let b_1_{x} := false;
        let p_1_{x} := null;
    
        let n_2_{x} : i64 = {x};
        let s_2_{x} : str = \"{x}\";
        let b_2_{x} : bool = false;
        let p_2_{x} : ^i64 = null;
        """)
    print("}")
