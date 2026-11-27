"""Draws figures for Experiments 6, 8 and 9 as SVGs (white background, the site frames them on a card).

  exp6/vending-machine.svg  the Experiment 6 Task's state machine: four states, the two buttons
  exp7/reaction-game.svg    the Experiment 7 Task's state machine: three states, buttons and a timer

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


# ------------------------------------------------------------------ state diagrams (Experiments 6 and 7)
SW1C, SW2C, TIMC = '#c2410c', '#7c3aed', '#0f766e'   # an arrow colour per kind of event

def state_diagram(path, w, h, label, nodes, edges, loops=(), start=None, key=(),
                  note='Each circle is a state, filled with its LED colour.'):
    """nodes: key -> (x, y, name, led text, fill, text colour, stroke)
    edges: (from, to, colour, label, bend, label dx, label dy, anchor, box)
    loops: (node, side, colour, label, anchor);  start: (node, side);  key: (colour, text) pairs"""
    import math
    s = Svg(w, h, label)
    R = 64

    def head(ex, ey, tx, ty, color):
        L = math.hypot(tx, ty); ux, uy = tx / L, ty / L
        bx, by = ex - ux * 14, ey - uy * 14
        s.add(f"<polygon points='{ex:.1f},{ey:.1f} {bx - uy * 7:.1f},{by + ux * 7:.1f} {bx + uy * 7:.1f},{by - ux * 7:.1f}' fill='{color}'/>")

    def tag(x, y, text, color, anchor='middle', box=True):
        if box:
            tw = len(text) * 7.4 + 14
            x0 = x - tw / 2 if anchor == 'middle' else (x if anchor == 'start' else x - tw)
            s.rect(x0, y - 12, tw, 24, '#ffffff', r=4)
        s.text(x, y, text, 13, color, anchor=anchor, weight='700')

    for a, b, color, text, bend, ldx, ldy, anchor, box in edges:
        x1, y1 = nodes[a][:2]; x2, y2 = nodes[b][:2]
        dx, dy = x2 - x1, y2 - y1
        L = math.hypot(dx, dy)
        cx, cy = (x1 + x2) / 2 - dy / L * bend, (y1 + y2) / 2 + dx / L * bend
        def trim(px, py, qx, qy):
            d = math.hypot(qx - px, qy - py)
            return px + (qx - px) / d * (R + 4), py + (qy - py) / d * (R + 4)
        sx, sy = trim(x1, y1, cx, cy); ex, ey = trim(x2, y2, cx, cy)
        s.add(f"<path d='M{sx:.1f},{sy:.1f} Q{cx:.1f},{cy:.1f} {ex:.1f},{ey:.1f}' fill='none' stroke='{color}' stroke-width='3'/>")
        head(ex, ey, ex - cx, ey - cy, color)
        tag((sx + 2 * cx + ex) / 4 + ldx, (sy + 2 * cy + ey) / 4 + ldy, text, color, anchor, box)

    for n, side, color, text, anchor in loops:
        x, y = nodes[n][:2]
        a = {'top': -90, 'right': 0, 'bottom': 90, 'left': 180}[side]
        pt = lambda deg, r: (x + r * math.cos(math.radians(deg)), y + r * math.sin(math.radians(deg)))
        sx, sy = pt(a - 28, R + 2); ex, ey = pt(a + 28, R + 6)
        c1x, c1y = pt(a - 34, R + 92); c2x, c2y = pt(a + 34, R + 92)
        s.add(f"<path d='M{sx:.1f},{sy:.1f} C{c1x:.1f},{c1y:.1f} {c2x:.1f},{c2y:.1f} {ex:.1f},{ey:.1f}' fill='none' stroke='{color}' stroke-width='3'/>")
        head(ex, ey, ex - c2x, ey - c2y, color)
        lx, ly = pt(a, R + 82 + (10 if side in ('left', 'right') else 0))
        tag(lx + (2 if anchor == 'start' else 0), ly - (2 if side == 'top' else 0), text, color, anchor, box=(anchor == 'middle'))

    if start:
        n, side = start
        x, y = nodes[n][:2]
        sgn = -1 if side == 'left' else 1
        s.add(f"<line x1='{x + sgn * (R + 90)}' y1='{y}' x2='{x + sgn * (R + 10)}' y2='{y}' stroke='#57606a' stroke-width='3'/>")
        head(x + sgn * (R + 4), y, -sgn, 0, '#57606a')
        s.text(x + sgn * (R + 96), y, 'start', 13, DIM, anchor='end' if sgn < 0 else 'start', weight='700')

    for n, (x, y, name, led, fill, fg, stroke) in nodes.items():
        s.add(f"<circle cx='{x}' cy='{y}' r='{R}' fill='{fill}' stroke='{stroke}' stroke-width='2.5'/>")
        s.text(x, y - 9, name, 16, fg, weight='800')
        s.text(x, y + 14, led, 12.5, fg)

    kx = 40
    for color, text in key:
        s.add(f"<line x1='{kx}' y1='{h - 28}' x2='{kx + 34}' y2='{h - 28}' stroke='{color}' stroke-width='3'/>")
        head(kx + 42, h - 28, 1, 0, color)
        s.text(kx + 52, h - 28, text, 13, INK, anchor='start')
        kx += 72 + len(text) * 7.6
    s.text(kx + 10, h - 28, note, 13, DIM, anchor='start')
    s.save(path)


OFF = ('#3a3f47', '#ffffff', '#1f2328')

def vending_machine(path):
    nodes = {'idle': (230, 170, 'Idle', 'LED off', *OFF),
             'one': (710, 170, 'One coin', 'blue', '#2563eb', '#ffffff', '#1e40af'),
             'ready': (710, 450, 'Ready', 'green', '#16a34a', '#ffffff', '#166534'),
             'vend': (230, 450, 'Vend', 'white', '#ffffff', INK, '#57606a')}
    edges = [('idle', 'one', SW1C, 'SW1: coin', -60, 0, -4, 'middle', True),
             ('one', 'idle', SW2C, 'SW2: refund', -60, 0, 4, 'middle', True),
             ('one', 'ready', SW1C, 'SW1: coin', 0, 14, 0, 'start', True),
             ('ready', 'vend', SW2C, 'SW2: dispense', 0, 0, 0, 'middle', True),
             ('vend', 'idle', SW2C, 'SW2: drink taken', 0, -14, 0, 'end', True),
             ('vend', 'one', SW1C, "SW1: next customer's coin", 0, 0, 0, 'middle', True)]
    loops = [('idle', 'top', SW2C, 'SW2: nothing to refund', 'middle'),
             ('ready', 'right', SW1C, 'SW1: coin refused', 'start')]
    state_diagram(path, 1000, 600, 'State diagram of the vending machine. Idle, LED off: SW1 goes to One coin; SW2 stays in Idle. One coin, blue: SW1 goes to Ready; SW2 refunds and goes back to Idle. Ready, green: SW2 dispenses and goes to Vend; SW1 stays in Ready, the coin is refused. Vend, white: SW1 goes to One coin; SW2 goes to Idle.',
                  nodes, edges, loops, start=('idle', 'left'),
                  key=((SW1C, 'SW1: insert a coin'), (SW2C, 'SW2: the button')))


def reaction_game(path):
    nodes = {'idle': (230, 330, 'Idle', 'off, red or blue', *OFF),
             'wait': (560, 150, 'Waiting', 'LED off', *OFF),
             'go': (890, 330, 'Go', 'green', '#16a34a', '#ffffff', '#166534')}
    edges = [('idle', 'wait', SW1C, 'SW1: start the random wait', -50, -30, -10, 'middle', True),
             ('wait', 'idle', SW2C, 'SW2 too early: false start, red', -50, 30, 14, 'middle', True),
             ('wait', 'go', TIMC, 'Timer1A runs out: green, note ms', -40, 40, -8, 'middle', True),
             ('go', 'idle', SW2C, 'SW2: reaction = ms - go_ms, blue', 0, 0, 0, 'middle', True)]
    state_diagram(path, 1120, 470, 'State diagram of the reaction game. Idle: SW1 starts a random wait and goes to Waiting. Waiting, LED off: when Timer1A runs out the LED turns green and the game goes to Go; SW2 before that is a false start: red, back to Idle. Go, green: SW2 stores the reaction time, turns the LED blue for a success, and goes back to Idle. Presses not shown are ignored.',
                  nodes, edges, start=('idle', 'left'),
                  key=((SW1C, 'SW1: start'), (SW2C, 'SW2: react'), (TIMC, 'Timer1A time-out')))


vending_machine('public/images/exp6/vending-machine.svg')
reaction_game('public/images/exp7/reaction-game.svg')
lcd_wiring('public/images/exp8/lcd-wiring.svg')
pot_wiring('public/images/exp9/pot-wiring.svg')
