# elliptic_curve.py

class Curve:
    def __init__(self, p, a, b, generator):
        self.p = p  # Prime modulus
        self.a = a  # Curve coefficient a
        self.b = b  # Curve coefficient b
        self.generator = Point(generator[0], generator[1], self)
    
    def is_on_curve(self, point):
        if point.infinity:
            return True
        x, y = point.x, point.y
        return (y * y - (x * x * x + self.a * x + self.b)) % self.p == 0

    def __repr__(self):
        return f"Curve over F_{self.p}: y^2 = x^3 + {self.a}x + {self.b}"


class Point:
    def __init__(self, x=None, y=None, curve=None, infinity=False):
        self.x = x
        self.y = y
        self.curve = curve
        self.infinity = infinity
    
    def __eq__(self, other):
        if self.infinity and other.infinity:
            return True
        if self.infinity or other.infinity:
            return False
        return self.x == other.x and self.y == other.y and self.curve == other.curve

    def __neg__(self):
        if self.infinity:
            return self
        return Point(self.x, (-self.y) % self.curve.p, self.curve)

    def __add__(self, Q):
        P = self
        if P.infinity:
            return Q
        if Q.infinity:
            return P

        if P.x == Q.x and (P.y != Q.y or P.y == 0):
            return Point(infinity=True, curve=self.curve)

        p = self.curve.p

        if P.x == Q.x:
            # Point doubling
            s_num = (3 * P.x * P.x + self.curve.a) % p
            s_den = modinv(2 * P.y, p)
        else:
            # Point addition
            s_num = (Q.y - P.y) % p
            s_den = modinv((Q.x - P.x) % p, p)

        s = (s_num * s_den) % p
        x_r = (s * s - P.x - Q.x) % p
        y_r = (s * (P.x - x_r) - P.y) % p
        return Point(x_r, y_r, self.curve)

    def __rmul__(self, k):
        result = Point(infinity=True, curve=self.curve)
        addend = self

        while k > 0:
            if k & 1:
                result = result + addend
            addend = addend + addend
            k >>= 1
        return result

    def __repr__(self):
        if self.infinity:
            return "Point(infinity)"
        return f"Point({self.x}, {self.y})"


def modinv(a, m):
    # Extended Euclidean Algorithm
    t, new_t = 0, 1
    r, new_r = m, a % m

    while new_r != 0:
        quotient = r // new_r
        t, new_t = new_t, t - quotient * new_t
        r, new_r = new_r, r - quotient * new_r

    if r != 1:
        raise ValueError(f"No inverse exists for {a} mod {m}")
    return t % m


# ======= Example Usage =======

if __name__ == "__main__":
    p = 20959
    a = 0
    b = 7
    Gx, Gy = 17263, 13626

    curve = Curve(p, a, b, generator=(Gx, Gy))
    G = curve.generator

    print("Elliptic Curve:", curve)
    print("Generator G:", G)
    print("Is G on the curve?", curve.is_on_curve(G))

    # Test scalar multiplication
    priv_key = 20942
    pub_key = priv_key * G

    print(f"{priv_key} * G = {pub_key}")
    print("Is pub_key on curve?", curve.is_on_curve(pub_key))

