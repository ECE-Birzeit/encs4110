"""Draws figures for Experiments 6, 8 and 9 as SVGs (white background, the site frames them on a card).

  exp6/vending-machine.svg  the Experiment 6 Task's state machine: four states, the two buttons

  exp8/lcd-wiring.svg     the 16x2 LCD: 5 V and GND rails, the contrast potentiometer, and every
                          signal straight up to its LaunchPad pin (header locations: setup/pinout)
  exp9/pot-wiring.svg     the potentiometer on AIN0 (PE3), between 3.3 V and GND

Run from the repo root:  python3 scripts/draw-figures.py"""
from html import escape

SANS = "ui-sans-serif, system-ui, -apple-system, 'Segoe UI', sans-serif"
MONO = "ui-monospace, 'SFMono-Regular', Menlo, Consolas, monospace"
INK, DIM, TEAL = '#1f2328', '#57606a', '#00878F'


class Svg:
    def __init__(self, w, h, label):
        self.w, self.h = w, h
        self.out = [f"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 {w} {h}' width='{w}' height='{h}' role='img' aria-label='{escape(label, quote=True)}'>",
                    "<defs><marker id='a' viewBox='0 0 10 10' refX='9' refY='5' markerWidth='7' markerHeight='7' orient='auto-start-reverse'><path d='M0,0 L10,5 L0,10 z' fill='#333'/></marker></defs>",
                    f"<rect width='{w}' height='{h}' fill='#fff'/>"]

    def add(self, s):
        self.out.append(s)

    def text(self, x, y, s, size=13, fill=INK, anchor='middle', weight='400', mono=False):
        f = MONO if mono else SANS
        self.add(f"<text x='{x:.1f}' y='{y:.1f}' text-anchor='{anchor}' dominant-baseline='central' font-family=\"{f}\" font-size='{size}' font-weight='{weight}' fill='{fill}'>{escape(s)}</text>")

    def rect(self, x, y, w, h, fill, stroke=None, r=0, sw=1.5, dash=None):
        st = f" stroke='{stroke}' stroke-width='{sw}'" if stroke else ''
        if dash:
            st += f" stroke-dasharray='{dash}'"
        self.add(f"<rect x='{x:.1f}' y='{y:.1f}' width='{w:.1f}' height='{h:.1f}' rx='{r}' fill='{fill}'{st}/>")

    def line(self, pts, color, w=3, dash=None, marker_end=False):
        d = f" stroke-dasharray='{dash}'" if dash else ''
        m = " marker-end='url(#a)'" if marker_end else ''
        self.add(f"<polyline fill='none' stroke='{color}' stroke-width='{w}' stroke-linejoin='round' stroke-linecap='round' points='{' '.join(f'{x:.1f},{y:.1f}' for x, y in pts)}'{d}{m}/>")

    def save(self, path):
        open(path, 'w').write(''.join(self.out) + '</svg>')
        print('wrote', path)


# ------------------------------------------------------------------ shared parts
WIRE = {'gnd': '#1f2328', 'pwr': '#d93025', 'p33': '#f08c00', 'ctl': '#d9a400', 'data': '#2e9d4b', 'pot': '#2563eb'}

def dot(s, x, y):
    s.add(f"<circle cx='{x:.1f}' cy='{y:.1f}' r='4.5' fill='#1f2328'/>")

def hop_up(s, x, y_from, y_to, y_rail, color):
    """A vertical wire from y_from up to y_to that hops over a rail at y_rail (no connection)."""
    r = 7
    s.add(f"<path d='M{x},{y_from} L{x},{y_rail + r} A{r},{r} 0 0 1 {x},{y_rail - r} L{x},{y_to}' fill='none' stroke='{color}' stroke-width='3' stroke-linecap='round'/>")

def trimmer(s, cx, top, legs, label='10k'):
    """A trimmer potentiometer seen from above: body, adjuster, value, and three legs going down."""
    s.rect(cx - 26, top, 52, 58, '#2b5fb8', r=6)
    s.add(f"<circle cx='{cx}' cy='{top + 20}' r='13' fill='#e8eef8'/>")
    s.add(f"<line x1='{cx - 9}' y1='{top + 20}' x2='{cx + 9}' y2='{top + 20}' stroke='#2b5fb8' stroke-width='3' stroke-linecap='round'/>")
    s.add(f"<line x1='{cx}' y1='{top + 11}' x2='{cx}' y2='{top + 29}' stroke='#2b5fb8' stroke-width='3' stroke-linecap='round'/>")
    s.text(cx, top + 46, label, 11, '#ffffff', weight='700')
    for x in legs:
        s.rect(x - 3, top + 58, 6, 10, '#9aa4ae', r=1)


# ------------------------------------------------------------------ Experiment 8: LCD wiring
LCD_PINS = ['VSS', 'VDD', 'V0', 'RS', 'RW', 'E', 'D0', 'D1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7', 'A', 'K']
# Signal pins straight up to their LaunchPad pins (pin -> (name, header pin, kind)); from setup/pinout
LCD_SIGNALS = {4: ('PB0', 'J1.3', 'ctl'), 5: ('PB1', 'J1.4', 'ctl'), 6: ('PB2', 'J2.2', 'ctl'),
               11: ('PB4', 'J1.7', 'data'), 12: ('PB5', 'J1.2', 'data'), 13: ('PB6', 'J2.7', 'data'), 14: ('PB7', 'J2.6', 'data')}

def lcd_wiring(path):
    s = Svg(1060, 800, 'Wiring the 16x2 LCD: a 5 V rail from VBUS and a GND rail; VSS and K to GND, VDD and A to 5 V; a 10 kilo-ohm potentiometer with its outer pins on 5 V and GND and its middle pin to V0; RS, RW and E to PB0, PB1 and PB2; D4 to D7 to PB4 to PB7; D0 to D3 not connected')
    x0, dx = 240, 46                    # LCD pin 1 and the pin pitch
    lcd_top = 570
    r5, rg = 90, 130                    # 5 V rail and GND rail
    px = lambda p: x0 + (p - 1) * dx

    # Rails
    for y, color, name in ((r5, WIRE['pwr'], '5 V'), (rg, WIRE['gnd'], 'GND')):
        s.line([(200, y), (960, y)], color, 4)
        s.text(188, y, name, 14, color, anchor='end', weight='700')

    # LaunchPad pins: one block, VBUS and GND feeding the rails from its top edge
    bx0, bx1, by0, by1 = 356, 866, 280, 396
    s.rect(bx0, by0, bx1 - bx0, by1 - by0, '#f4d6d6', '#c99a9a', r=10)
    s.text(470, 310, 'LaunchPad', 15, INK, anchor='middle', weight='700')
    s.text(470, 332, 'header pins', 12, DIM)
    for x, name, where, color, rail in ((700, 'VBUS', 'J3.1', WIRE['pwr'], r5), (770, 'GND', 'J3.2', WIRE['gnd'], rg)):
        s.text(x, by0 + 22, name, 12.5, INK, weight='700', mono=True)
        s.text(x, by0 + 39, where, 11, DIM, mono=True)
        s.rect(x - 6, by0 - 6, 12, 12, '#d9a400', r=2)
        if rail == r5:
            hop_up(s, x, by0 - 6, r5, rg, color)
        else:
            s.line([(x, by0 - 6), (x, rg)], color)
        dot(s, x, rail)

    # The LCD's signal pins, each straight up to its pad
    for p, (name, where, kind) in LCD_SIGNALS.items():
        x = px(p)
        s.line([(x, lcd_top + 6), (x, by1 + 6)], WIRE[kind])
        s.rect(x - 6, by1 - 6, 12, 12, '#d9a400', r=2)
        s.text(x, by1 - 40, name, 12.5, INK, weight='700', mono=True)
        s.text(x, by1 - 23, where, 11, DIM, mono=True)

    # Power pins to the rails
    s.line([(px(1), lcd_top + 6), (px(1), rg)], WIRE['gnd']); dot(s, px(1), rg)                  # VSS
    hop_up(s, px(2), lcd_top + 6, r5, rg, WIRE['pwr']); dot(s, px(2), r5)                         # VDD
    hop_up(s, px(15), lcd_top + 6, r5, rg, WIRE['pwr']); dot(s, px(15), r5)                       # A
    s.line([(px(16), lcd_top + 6), (px(16), rg)], WIRE['gnd']); dot(s, px(16), rg)                # K

    # The contrast potentiometer, right above V0: outer pins on the rails, wiper straight down
    # (all three wires leave from the legs at the bottom; the outer two bend out and go up)
    # It sits between the rails and the LaunchPad block, so its rail wires stay short
    cx, ptop = px(3), 158
    legs = (cx - 14, cx, cx + 14)
    leg_end, bend = ptop + 68, ptop + 80
    xl, xr = px(2) + 10, px(3) + 40      # left of the pot (right of VDD), and right of the pot
    s.line([(legs[0], leg_end), (legs[0], bend), (xl, bend)], WIRE['pwr'])
    hop_up(s, xl, bend, r5, rg, WIRE['pwr']); dot(s, xl, r5)
    s.line([(legs[2], leg_end), (legs[2], bend), (xr, bend), (xr, rg)], WIRE['gnd']); dot(s, xr, rg)
    s.line([(legs[1], leg_end), (legs[1], lcd_top + 6)], WIRE['pot'])
    trimmer(s, cx, ptop, legs)
    s.text(xr + 16, ptop + 20, 'contrast', 12, INK, anchor='start', weight='700')
    s.text(xr + 16, ptop + 37, 'potentiometer', 11.5, DIM, anchor='start')

    # The LCD module
    s.rect(190, lcd_top, 790, 200, '#2f6b3a', r=10)
    s.rect(236, lcd_top + 74, 698, 104, '#1b2b20', r=6)
    s.rect(250, lcd_top + 86, 670, 80, '#8bc34a', r=3)
    for p in range(1, 17):
        x = px(p)
        nc = 7 <= p <= 10
        s.add(f"<circle cx='{x}' cy='{lcd_top + 14}' r='8' fill='{'#9aa4ae' if nc else '#c9a24a'}' stroke='#7a5d1c' stroke-width='1.5'/>")
        s.text(x, lcd_top + 14, str(p), 8.5, '#2b2f36', weight='800')
        s.text(x, lcd_top + 42, LCD_PINS[p - 1], 12, '#ffffff', weight='700', mono=True)
    s.text(585, lcd_top + 126, '16x2 LCD (HD44780)', 15, '#1b2b20', weight='700')
    s.text((px(7) + px(10)) / 2, lcd_top - 30, 'D0-D3', 11.5, DIM, weight='700')
    s.text((px(7) + px(10)) / 2, lcd_top - 15, 'not connected', 11.5, DIM)

    s.text(530, 790, 'Turn the potentiometer to set the contrast. Where a wire hops over the GND rail, it does not connect to it.', 12.5, DIM)
    s.save(path)


# ------------------------------------------------------------------ Experiment 9: potentiometer on AIN0
def pot_wiring(path):
    s = Svg(820, 400, 'The potentiometer on the LaunchPad: one outer pin to 3.3 V on J1 pin 1, the middle pin (wiper) to PE3, which is AIN0, on J3 pin 9, and the other outer pin to GND on J3 pin 2')
    # LaunchPad pins on the right
    bx0, by0, bx1, by1 = 520, 100, 780, 320
    s.rect(bx0, by0, bx1 - bx0, by1 - by0, '#f4d6d6', '#c99a9a', r=12)
    s.text(650, 126, 'LaunchPad', 15, INK, weight='700')
    pins = [(150, '3.3V', 'J1.1', WIRE['p33']), (210, 'PE3 (AIN0)', 'J3.9', WIRE['pot']), (270, 'GND', 'J3.2', WIRE['gnd'])]
    for y, name, where, _ in pins:
        s.rect(bx0 - 6, y - 6, 12, 12, '#d9a400', r=2)
        s.text(bx0 + 22, y - 8, name, 13, INK, anchor='start', weight='700', mono=True)
        s.text(bx0 + 22, y + 10, where, 11.5, DIM, anchor='start', mono=True)
    # The potentiometer, legs pointing right
    cx, cy = 190, 210
    s.rect(cx - 50, cy - 60, 80, 120, '#2b5fb8', r=8)
    s.add(f"<circle cx='{cx - 10}' cy='{cy}' r='22' fill='#e8eef8'/>")
    s.add(f"<line x1='{cx - 24}' y1='{cy}' x2='{cx + 4}' y2='{cy}' stroke='#2b5fb8' stroke-width='4' stroke-linecap='round'/>")
    s.text(cx - 10, cy + 84, '10 kΩ potentiometer', 13, INK, weight='700')
    for (y, name, where, color), leg in zip(pins, ('outer pin', 'wiper', 'outer pin')):
        ly = cy + (y - 210) * 0.6
        s.rect(cx + 30, ly - 3, 14, 6, '#9aa4ae', r=1)
        s.line([(cx + 44, ly), (420, ly), (420, y) if ly != y else (420, ly), (bx0 - 6, y)], color)
        s.text(cx + 52, ly - 12, leg, 11, DIM, anchor='start')
    s.text(410, 375, 'Use 3.3V, not VBUS: PE3 must never see more than 3.3 V.', 12.5, DIM)
    s.save(path)


# ------------------------------------------------------------------ Experiment 6: the vending machine
def vending_machine(path):
    import math
    SW1C, SW2C = '#c2410c', '#7c3aed'           # one colour per button, for arrows and labels
    s = Svg(1000, 600, 'State diagram of the vending machine. Idle, LED off: SW1 goes to One coin; SW2 stays in Idle. One coin, blue: SW1 goes to Ready; SW2 refunds and goes back to Idle. Ready, green: SW2 dispenses and goes to Vend; SW1 stays in Ready, the coin is refused. Vend, white: SW1 goes to One coin; SW2 goes to Idle.')
    def head(ex, ey, tx, ty, color):
        """A filled arrowhead with its tip at (ex, ey), pointing along the direction (tx, ty)."""
        L = math.hypot(tx, ty); ux, uy = tx / L, ty / L
        bx, by = ex - ux * 14, ey - uy * 14
        s.add(f"<polygon points='{ex:.1f},{ey:.1f} {bx - uy * 7:.1f},{by + ux * 7:.1f} {bx + uy * 7:.1f},{by - ux * 7:.1f}' fill='{color}'/>")
    R = 64
    N = {'idle': (230, 170), 'one': (710, 170), 'ready': (710, 450), 'vend': (230, 450)}
    def edge(a, b, color, marker, bend=0.0):
        (x1, y1), (x2, y2) = N[a], N[b]
        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
        dx, dy = x2 - x1, y2 - y1
        L = math.hypot(dx, dy)
        cx, cy = mx - dy / L * bend, my + dx / L * bend          # control point, off to one side
        def trim(px, py, qx, qy):                               # move from p toward q by R (+ a gap)
            d = math.hypot(qx - px, qy - py)
            return px + (qx - px) / d * (R + 4), py + (qy - py) / d * (R + 4)
        sx, sy = trim(x1, y1, cx, cy)
        ex, ey = trim(x2, y2, cx, cy)
        s.add(f"<path d='M{sx:.1f},{sy:.1f} Q{cx:.1f},{cy:.1f} {ex:.1f},{ey:.1f}' fill='none' stroke='{color}' stroke-width='3'/>")
        head(ex, ey, ex - cx, ey - cy, color)
        return (sx + 2 * cx + ex) / 4, (sy + 2 * cy + ey) / 4    # the curve's midpoint, for its label
    def loop(n, side, color, marker):
        x, y = N[n]
        a = {'top': -90, 'right': 0}[side]
        p = lambda deg, r: (x + r * math.cos(math.radians(deg)), y + r * math.sin(math.radians(deg)))
        sx, sy = p(a - 28, R + 2); ex, ey = p(a + 28, R + 6)
        c1x, c1y = p(a - 34, R + 92); c2x, c2y = p(a + 34, R + 92)
        s.add(f"<path d='M{sx:.1f},{sy:.1f} C{c1x:.1f},{c1y:.1f} {c2x:.1f},{c2y:.1f} {ex:.1f},{ey:.1f}' fill='none' stroke='{color}' stroke-width='3'/>")
        head(ex, ey, ex - c2x, ey - c2y, color)
        return p(a, R + 82)
    def label(x, y, btn, what, color, anchor='middle', box=True):
        t = f'{btn}' + (f': {what}' if what else '')
        if box:
            w = len(t) * 7.4 + 14
            x0 = x - w / 2 if anchor == 'middle' else (x if anchor == 'start' else x - w)
            s.rect(x0, y - 12, w, 24, '#ffffff', r=4)
        s.text(x if anchor != 'middle' else x, y, t, 13, color, anchor=anchor, weight='700')

    # Start arrow into Idle
    x, y = N['idle']
    s.add(f"<line x1='{x - R - 90}' y1='{y}' x2='{x - R - 10}' y2='{y}' stroke='#57606a' stroke-width='3'/>")
    head(x - R - 4, y, 1, 0, '#57606a')
    s.text(x - R - 96, y, 'start', 13, DIM, anchor='end', weight='700')

    mx, my = edge('idle', 'one', SW1C, 'm1', bend=-60);  label(mx, my - 4, 'SW1', 'coin', SW1C)
    mx, my = edge('one', 'idle', SW2C, 'm2', bend=-60);  label(mx, my + 4, 'SW2', 'refund', SW2C)
    mx, my = edge('one', 'ready', SW1C, 'm1');           label(mx + 14, my, 'SW1', 'coin', SW1C, anchor='start')
    mx, my = edge('ready', 'vend', SW2C, 'm2');          label(mx, my, 'SW2', 'dispense', SW2C)
    mx, my = edge('vend', 'idle', SW2C, 'm2');           label(mx - 14, my, 'SW2', 'drink taken', SW2C, anchor='end')
    mx, my = edge('vend', 'one', SW1C, 'm1');            label(mx, my, 'SW1', "next customer's coin", SW1C)
    lx, ly = loop('idle', 'top', SW2C, 'm2');            label(lx, ly - 2, 'SW2', 'nothing to refund', SW2C)
    lx, ly = loop('ready', 'right', SW1C, 'm1');         label(lx + 2, ly, 'SW1', 'coin refused', SW1C, anchor='start', box=False)

    # The states, filled with their LED colour
    for key, name, led, fill, fg, stroke in (
            ('idle', 'Idle', 'LED off', '#3a3f47', '#ffffff', '#1f2328'),
            ('one', 'One coin', 'blue', '#2563eb', '#ffffff', '#1e40af'),
            ('ready', 'Ready', 'green', '#16a34a', '#ffffff', '#166534'),
            ('vend', 'Vend', 'white', '#ffffff', INK, '#57606a')):
        x, y = N[key]
        s.add(f"<circle cx='{x}' cy='{y}' r='{R}' fill='{fill}' stroke='{stroke}' stroke-width='2.5'/>")
        s.text(x, y - 9, name, 16, fg, weight='800')
        s.text(x, y + 14, led, 12.5, fg)

    # Key
    s.add(f"<line x1='40' y1='572' x2='74' y2='572' stroke='{SW1C}' stroke-width='3'/>"); head(82, 572, 1, 0, SW1C)
    s.text(90, 572, 'SW1: insert a coin', 13, INK, anchor='start')
    s.add(f"<line x1='290' y1='572' x2='324' y2='572' stroke='{SW2C}' stroke-width='3'/>"); head(332, 572, 1, 0, SW2C)
    s.text(340, 572, 'SW2: the button', 13, INK, anchor='start')
    s.text(520, 572, 'Each circle is a state, filled with its LED colour.', 13, DIM, anchor='start')
    s.save(path)


vending_machine('public/images/exp6/vending-machine.svg')
lcd_wiring('public/images/exp8/lcd-wiring.svg')
pot_wiring('public/images/exp9/pot-wiring.svg')
