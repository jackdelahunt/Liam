main_code = """
    fn main() void {

    }
"""

print(main_code)

for i in range(3000):
    function_code = f"""fn add_{i}(a: i64, b: i64, c: i64, d: bool, e: str) void {{
            a = b;
            b = a;
            c = b;
            a = c;

            if d {{
                e = "this is a new string";
            }} else {{
                e = "this is the else string";
            }}
        }}"""
    print(function_code)
