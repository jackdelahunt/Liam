main_code = """
    fn main() void {

    }
"""

print(main_code)

body_code = ""

for i in range(90000):
    body_code = f"""
    let n_1_{i} := {i};
    let s_1_{i} := \"{i}\";
    let b_1_{i} := false;
    let p_1_{i} := null;
   
    let n_2_{i} : i64 = {i};
    let s_2_{i} : str = \"{i}\";
    let b_2_{i} : bool = false;
    let p_2_{i} : ^i64 = null;
    {body_code}
    """

for i in range(10):
    function_code = f"""fn add_{i}(a: i64, b: i64, c: i64, d: bool, e: str) void {{
        {body_code}
    }}"""
    print(function_code)
