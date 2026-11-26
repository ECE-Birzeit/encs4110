"""Draws the EK-TM4C123GXL LaunchPad's BoosterPack headers as SVGs, top and bottom view.

Pin data: TI SPMU296 (EK-TM4C123GXL user's guide), the J1-J4 connector tables, as checked
against the PDF in the TM4C123 simulator's CI (apps/web/board/ek-tm4c123gxl.pins.json).
The drawing style follows encs4101's scripts/draw-pinouts.py, so the lab sites match.

Top view: J1 (outer) and J3 (inner) on the left, J4 (inner) and J2 (outer) on the right,
pin 1 of every header at the USB end. The bottom view flips the board left-to-right.

Run from the repo root:  python3 scripts/draw-pinout.py
Writes public/images/pinout/launchpad-top.svg and launchpad-bottom.svg."""
import sys
from html import escape

OUT_DIR = sys.argv[1] if len(sys.argv) > 1 else 'public/images/pinout'

# category -> (fill, text colour)
CAT = {
    'gnd':   ('#1f2328', '#ffffff'),
    'pwr':   ('#c62828', '#ffffff'),
    'free':  ('#f08c00', '#ffffff'),
    'board': ('#0f8f80', '#ffffff'),
    'dbg':   ('#8d6e63', '#ffffff'),
}
TAG = {  # tag kind -> (fill, stroke, text)
    'use':  ('#d7f1ec', '#0f8f80', '#0b5f55'),
    'adc':  ('#fde9a8', '#d9a400', '#6b5100'),
    'tim':  ('#ffffff', '#f08c00', '#a35e00'),
    'com':  ('#e6e3fb', '#7b6fd6', '#40368f'),
    'qei':  ('#dbeafe', '#2563eb', '#1e3a8a'),
    'warn': ('#fde2e1', '#c62828', '#8e1b1b'),
}

def pin(name, cat, *tags):
    return (name, cat, list(tags))

def g(name, *tags):  # free GPIO
    return pin(name, 'free', *tags)

def b(name, *tags):  # GPIO wired to an on-board part
    return pin(name, 'board', *tags)

GND = pin('GND', 'gnd')

# Each block is ten rows of (outer header pin, inner header pin), pin 1 first.
J1 = [pin('3.3V', 'pwr'),
      g('PB5', ('adc', 'AIN11'), ('tim', 'T1CCP1')),
      g('PB0', ('com', 'U1Rx'), ('tim', 'T2CCP0'), ('warn', '3.3 V only')),
      g('PB1', ('com', 'U1Tx'), ('tim', 'T2CCP1'), ('warn', '3.3 V only')),
      g('PE4', ('adc', 'AIN9'), ('com', 'I2C2SCL')),
      g('PE5', ('adc', 'AIN8'), ('com', 'I2C2SDA')),
      g('PB4', ('adc', 'AIN10'), ('tim', 'T1CCP0')),
      g('PA5', ('com', 'SSI0Tx')),
      g('PA6', ('com', 'I2C1SCL')),
      g('PA7', ('com', 'I2C1SDA'))]
J3 = [pin('5V', 'pwr'),
      GND,
      g('PD0', ('adc', 'AIN7'), ('com', 'I2C3SCL'), ('warn', '= PB6 (R9)')),
      g('PD1', ('adc', 'AIN6'), ('com', 'I2C3SDA'), ('warn', '= PB7 (R10)')),
      g('PD2', ('adc', 'AIN5')),
      g('PD3', ('adc', 'AIN4'), ('qei', 'IDX0')),
      g('PE1', ('adc', 'AIN2'), ('com', 'U7Tx')),
      g('PE2', ('adc', 'AIN1')),
      g('PE3', ('adc', 'AIN0')),
      b('PF1', ('use', 'RED LED'), ('tim', 'T0CCP1'))]
J4 = [b('PF2', ('use', 'BLUE LED'), ('tim', 'T1CCP0')),
      b('PF3', ('use', 'GREEN LED'), ('tim', 'T1CCP1')),
      g('PB3', ('com', 'I2C0SDA'), ('tim', 'T3CCP1')),
      g('PC4', ('qei', 'IDX1'), ('com', 'U4Rx')),
      g('PC5', ('qei', 'PhA1'), ('com', 'U4Tx')),
      g('PC6', ('qei', 'PhB1'), ('com', 'U3Rx')),
      g('PC7', ('com', 'U3Tx'), ('tim', 'WT1CCP1')),
      g('PD6', ('qei', 'PhA0'), ('com', 'U2Rx')),
      g('PD7', ('qei', 'PhB0'), ('com', 'U2Tx'), ('warn', 'locked')),
      b('PF4', ('use', 'SW1'), ('tim', 'T2CCP0'))]
J2 = [GND,
      g('PB2', ('com', 'I2C0SCL'), ('tim', 'T3CCP0')),
      g('PE0', ('adc', 'AIN3'), ('com', 'U7Rx')),
      b('PF0', ('use', 'SW2'), ('tim', 'T0CCP0'), ('warn', 'locked')),
      pin('RST', 'dbg'),
      g('PB7', ('tim', 'T0CCP1'), ('warn', '= PD1 (R10)')),
      g('PB6', ('tim', 'T0CCP0'), ('warn', '= PD0 (R9)')),
      g('PA4', ('com', 'SSI0Rx')),
      g('PA3', ('com', 'SSI0Fss')),
      g('PA2', ('com', 'SSI0Clk'))]

FONT = "ui-monospace, 'SFMono-Regular', Menlo, Consolas, 'Liberation Mono', monospace"
ROW = 40
TOP = 220
NAME_W = 62
CH = 7.3  # approx char width at 12px mono
W = 2200

out = []

def add(s):
    out.append(s)

def tag_w(label):
    return round(len(label) * CH + 16)

def pill(x, y, w, h, fill, stroke=None, r=None, dash=None):
    r = h / 2 if r is None else r
    s = f' stroke="{stroke}" stroke-width="1.5"' if stroke else ''
    if dash:
        s += f' stroke-dasharray="{dash}"'
    add(f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h}" rx="{r}" fill="{fill}"{s}/>')

def text(x, y, s, size=12, fill='#1f2328', anchor='middle', weight='600'):
    add(f'<text x="{x:.1f}" y="{y:.1f}" font-family="{FONT}" font-size="{size}" font-weight="{weight}" fill="{fill}" text-anchor="{anchor}" dominant-baseline="central">{escape(s)}</text>')

def name_pill(cx, cy, p):
    name, cat, _ = p
    fill, fg = CAT[cat]
    pill(cx - NAME_W / 2, cy - 11, NAME_W, 22, fill)
    text(cx, cy, name, 13, fg, weight='700')

def tags(x, cy, p, direction):
    """Draw a pin's tags starting at x, going left (-1) or right (+1)."""
    for kind, label in p[2]:
        w = tag_w(label)
        fill, stroke, fg = TAG[kind]
        left = x - w if direction < 0 else x
        pill(left, cy - 10, w, 20, fill, stroke, r=4)
        text(left + w / 2, cy, label, 11.5, fg)
        x += direction * (w + 6)

def block(cx, left_col, right_col, left_name, right_name):
    """Two 10-pin headers side by side, as on the board: one column each, pin 1 at the top."""
    dx = 13
    y0, y1 = TOP - 18, TOP + 9 * ROW + 18
    pill(cx - 2 * dx, y0, 4 * dx, y1 - y0, '#2b2f36', r=6)
    text(cx - dx, y0 - 20, left_name, 17, '#1f2328', weight='800')
    text(cx + dx, y0 - 20, right_name, 17, '#1f2328', weight='800')
    for i in range(10):
        cy = TOP + i * ROW
        for x in (cx - dx, cx + dx):
            if i == 0:  # pin 1 is the square pad
                add(f'<rect x="{x - 7:.1f}" y="{cy - 7:.1f}" width="14" height="14" fill="#d9a400"/>')
            else:
                add(f'<circle cx="{x:.1f}" cy="{cy}" r="6.5" fill="#d9a400"/>')
            text(x, cy, str(i + 1), 7.5, '#2b2f36', weight='800')
        lx = cx - 2 * dx - 10 - NAME_W / 2
        rx = cx + 2 * dx + 10 + NAME_W / 2
        name_pill(lx, cy, left_col[i])
        name_pill(rx, cy, right_col[i])
        tags(lx - NAME_W / 2 - 6, cy, left_col[i], -1)
        tags(rx + NAME_W / 2 + 6, cy, right_col[i], +1)

# Part positions from TI's SPMR240 design files (mil, y up, board 2000 x 2600), as extracted
# in the TM4C123 simulator's CI (apps/web/board/launchpad.js, CAD_PLACEMENT).
BOARD = (2000, 2600)
PARTS = {
    'J1': (100, 1400), 'J3': (200, 1400), 'J4': (1800, 1400), 'J2': (1900, 1400),  # pin 1; pins run down
    'U1': (1000, 1000),     # TM4C123GH6PM, the target
    'U2': (1200, 2230),     # ICDI: the on-board debug probe
    'J11': (616, 2540),     # debug USB (micro-B), top edge
    'J9': (60, 1800),       # device USB (micro-AB), left edge
    'SW3': (200, 2400),     # power select switch
    'D4': (772, 2200),      # power LED
    'D1': (1904, 1616),     # RGB LED
    'RESET': (1770, 1880),
    'SW1': (400, 200), 'SW2': (1600, 200),
    'R9': (814, 633), 'R10': (942, 633),
}
S = 0.2  # sketch pixels per mil

def sketch(view, bx, by):
    """The LaunchPad to scale, so the headers, LEDs, buttons and debug probe are where they are on
    the real board. The bottom view is mirrored and shows only what you see from below."""
    bw, bh = BOARD[0] * S, BOARD[1] * S
    mirror = view == 'bottom'
    def at(name, dx=0, dy=0):
        x, y = PARTS[name]
        x = BOARD[0] - x if mirror else x
        return bx + x * S + dx, by + (BOARD[1] - y) * S + dy
    dash = '8 6' if mirror else None
    pill(bx, by, bw, bh, '#f4d6d6' if not mirror else '#efe4e4', '#c99a9a', r=10, dash=dash)
    text(bx + bw / 2, by - 44, f'board, {view} view (to scale)', 13, '#4b5158')

    # USB connectors stick out past the edge, so they show from both sides
    x, y = at('J11')
    pill(x - 32, y - 26, 64, 34, '#9aa4ae', r=4)
    text(x, y + 22, 'DEBUG USB', 10.5, '#1f2328', weight='700')
    x, y = at('J9')
    pill(x - (22 if not mirror else 14), y - 30, 36, 60, '#9aa4ae', r=4)
    text(x + (44 if not mirror else -44), y, 'DEVICE USB', 10.5, '#1f2328', anchor='start' if not mirror else 'end', weight='700')

    # Headers: two columns of ten, pin 1 at the top (the USB end)
    for name in ('J1', 'J3', 'J4', 'J2'):
        x, y = at(name)
        pill(x - 9, y - 10, 18, 900 * S + 20, '#2b2f36', r=3)
        add(f'<rect x="{x - 5:.1f}" y="{y - 5:.1f}" width="10" height="10" fill="#d9a400"/>')
        text(x, y - 20, name, 11, '#1f2328', weight='800')

    if mirror:
        lines = ['Seen from below,', 'flipped left-to-right:', 'J2 and J4 on the left,', 'J3 and J1 on the right.',
                 '', 'Pin 1 is still at', 'the USB end.', '', 'LEDs, buttons, and', 'chips are on the', 'other side.']
        for i, s_ in enumerate(lines):
            text(bx + bw / 2, by + 250 + i * 22, s_, 12.5, '#4b5158')
        return

    x, y = at('SW3')
    pill(x - 16, y - 8, 32, 16, '#4b5158', r=3)
    text(x, y + 20, 'PWR SELECT', 9.5, '#4b5158')
    x, y = at('D4')
    add(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="5" fill="#2e9d4b"/>')
    text(x, y + 16, 'PWR', 9.5, '#4b5158')
    x, y = at('U2')
    pill(x - 40, y - 40, 80, 80, '#2b2f36', r=4)
    text(x, y - 8, 'ICDI', 12, '#ffffff', weight='800')
    text(x, y + 10, 'debug probe', 9.5, '#c3c7cc')
    x, y = at('RESET')
    add(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="11" fill="#1f2328"/>')
    text(x - 18, y, 'RESET', 10, '#1f2328', anchor='end', weight='700')
    x, y = at('D1')
    for dy, col in ((-7, '#d93025'), (0, '#1a73e8'), (7, '#2e9d4b')):
        add(f'<circle cx="{x - 2:.1f}" cy="{y + dy:.1f}" r="3.6" fill="{col}"/>')
    text(x - 14, y - 6, 'RGB LED', 10, '#1f2328', anchor='end', weight='700')
    text(x - 14, y + 8, 'PF1-PF3', 9.5, '#4b5158', anchor='end')
    x, y = at('U1')
    add(f'<rect x="{x - 40:.1f}" y="{y - 40:.1f}" width="80" height="80" rx="4" fill="#2b2f36" transform="rotate(45 {x:.1f} {y:.1f})"/>')
    text(x, y - 6, 'TM4C123', 11, '#ffffff', weight='800')
    text(x, y + 10, 'GH6PM', 9.5, '#c3c7cc')
    xa, ya = at('R9')
    xb, _ = at('R10')
    for xx in (xa, xb):
        pill(xx - 7, ya - 4, 14, 8, '#8d6e63', r=2)
    text((xa + xb) / 2, ya + 16, 'R9 R10', 9.5, '#8e1b1b', weight='700')
    for name, lbl in (('SW1', 'SW1 · PF4'), ('SW2', 'SW2 · PF0')):
        x, y = at(name)
        add(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="12" fill="#1f2328"/>')
        text(x, y - 22, lbl, 10.5, '#1f2328', weight='700')

VIEWS = {'top': 'top view (component side)', 'bottom': 'bottom view (solder side)'}
NOTES = [
    'Pin 1 of each header is at the USB end. Pin names are also printed on the board.',
    'PB6/PD0 and PB7/PD1 are joined by R9/R10: use only one of each pair, or remove the resistor.',
    'PF0 and PD7 are locked (NMI): unlock with GPIO LOCK and CR before changing them.',
    'GPIOs are 3.3 V. Inputs tolerate 5 V, except PB0, PB1, PD4 and PD5 (3.6 V max).',
]

def draw(view):
    out.clear()
    legend_y = TOP + 10 * ROW + 110
    H = legend_y + 150 + len(NOTES) * 30 + 50
    add(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}">')
    add(f'<rect width="{W}" height="{H}" fill="#ffffff"/>')
    text(60, 62, 'EK-TM4C123GXL LaunchPad', 34, '#1f2328', anchor='start', weight='800')
    text(60, 98, f'TM4C123GH6PM · BoosterPack headers J1-J4, {VIEWS[view]}', 16, '#4b5158', anchor='start')
    text(W - 60, 62, 'ENCS4110', 22, '#dc2626', anchor='end', weight='800')
    text(W - 60, 92, 'encs4110.birzeit.me/setup/pinout', 13, '#4b5158', anchor='end')

    if view == 'top':
        block(560, J1, J3, 'J1', 'J3')
        block(1640, J4, J2, 'J4', 'J2')
    else:
        block(560, J2, J4, 'J2', 'J4')
        block(1640, J3, J1, 'J3', 'J1')
    sketch(view, 900, TOP - 60)

    add(f'<rect x="0" y="{legend_y - 40}" width="{W}" height="{H - legend_y + 40}" fill="#f3f5f7"/>')
    items = [('gnd', 'Ground'), ('pwr', 'Power'), ('free', 'Free GPIO'),
             ('board', 'GPIO wired on the board'), ('dbg', 'Reset')]
    for i, (cat, lbl) in enumerate(items):
        x, y = 60 + (i // 3) * 330, legend_y + (i % 3) * 38
        pill(x, y - 11, 46, 22, CAT[cat][0])
        text(x + 62, y, lbl, 15, '#1f2328', anchor='start')
    tagitems = [('use', 'RED LED', 'On-board part'), ('adc', 'AIN0', 'ADC input'),
                ('tim', 'T1CCP0', 'Timer pin (PWM, capture)'), ('com', 'I2C0SDA', 'UART / I2C / SSI'),
                ('qei', 'PhA1', 'Quadrature encoder'), ('warn', 'locked', 'Take care')]
    for i, (kind, sample, lbl) in enumerate(tagitems):
        x, y = 760 + (i // 3) * 440, legend_y + (i % 3) * 38
        fill, stroke, fg = TAG[kind]
        w = tag_w(sample)
        pill(x, y - 10, w, 20, fill, stroke, r=4)
        text(x + w / 2, y, sample, 11.5, fg)
        text(x + w + 14, y, lbl, 15, '#1f2328', anchor='start')
    for i, s in enumerate(NOTES):
        text(60, legend_y + 130 + i * 30, s, 14, '#1f2328', anchor='start')
    text(60, H - 30, 'Source: TI SPMU296 (EK-TM4C123GXL user\'s guide), BoosterPack connector tables. Drawn for ENCS4110, Birzeit University.', 12, '#6b7178', anchor='start', weight='500')
    add('</svg>')
    return '\n'.join(out)

for view in VIEWS:
    path = f'{OUT_DIR}/launchpad-{view}.svg'
    open(path, 'w').write(draw(view))
    print('wrote', path)
