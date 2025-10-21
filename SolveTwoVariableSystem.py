def solve_two_variables_system(coeffs1, coeffs2, n):
    """
    Solve system:
    a1*x + b1*L ≡ c1 (mod n)
    a2*x + b2*L ≡ c2 (mod n)
    """
    a1, b1, c1 = coeffs1
    a2, b2, c2 = coeffs2
    
    # Solve using elimination
    # Multiply first eq by b2, second eq by b1
    # Then subtract to eliminate L
    coeff_x = (a1 * b2 - a2 * b1) % n
    rhs = (c1 * b2 - c2 * b1) % n
    
    try:
        inv_coeff_x = pow(coeff_x, -1, n)
        x = (rhs * inv_coeff_x) % n
        
        # Now solve for L using first equation
        # b1*L ≡ c1 - a1*x (mod n)
        rhs_L = (c1 - a1 * x) % n
        inv_b1 = pow(b1, -1, n)
        L = (rhs_L * inv_b1) % n
        
        # Verify both equations
        eq1 = (a1 * x + b1 * L) % n
        eq2 = (a2 * x + b2 * L) % n
        
        print(f"Solution:")
        print(f"x = {x}")
        print(f"L = {L}")
        print(f"Verification:")
        print(f"Eq1: {a1}*{x} + {b1}*{L} = {eq1} (should be {c1})")
        print(f"Eq2: {a2}*{x} + {b2}*{L} = {eq2} (should be {c2})")
        
        return x, L
        
    except ValueError:
        print(f"No solution: coefficient {coeff_x} has no inverse mod {n}")
        return None, None

# Your system:
# 11602*x + 1*L ≡ -14664 (mod 20947)
# 19034*x + 1*L ≡ -6894  (mod 20947)
n = 20947
coeffs1 = [11602, 1, (-14664) % n]  # [a1, b1, c1]
coeffs2 = [19034, 1, (-6894) % n]   # [a2, b2, c2]

x, L = solve_two_variables_system(coeffs1, coeffs2, n)
