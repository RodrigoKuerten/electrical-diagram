from PIL import Image, ImageDraw, ImageFont
import os, glob

root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
cad = os.path.join(root, "docs", "cadesimu", "symbols")
ours = os.path.join(root, "build", "symbol_pngs")
out = os.path.join(root, "docs", "cadesimu", "visual-comparison")
os.makedirs(out, exist_ok=True)

pairs = [
    ("32834_Contactor_III", "contactor_3p"),
    ("32843_Fusible_IN", "fuse_1p"),
    ("32829_Motor_trif_sico", "motor_3ph"),
    ("32910_Pulsador_NA", "pushbutton_no"),
    ("32953_Temporizador_a_la_conexi_n", "timer_on"),
    ("32958_Piloto_se_alizaci_n", "pilot_lamp"),
    ("32969_Borne_cuadrado", "terminal_square"),
    ("33021_Det_inductivo_NC", "sensor_inductive_nc"),
    ("33044_Diodo", "diode"),
    ("33057_AND", "logic_and"),
    ("33338_S7_1200_8DI_8DO", "plc_s7_1200"),
    ("32957_Bobina_bistable_contactor_rel", "contactor_coil_bistable"),
]

for prefix, ours_type in pairs:
    cad_files = glob.glob(os.path.join(cad, prefix + "_x4.png"))
    our_file = os.path.join(ours, ours_type + ".svg.png")
    if not cad_files or not os.path.exists(our_file):
        print("missing", prefix, ours_type)
        continue
    left = Image.open(cad_files[0]).convert("RGB")
    right = Image.open(our_file).convert("RGB")
    h = 360
    def fit(im):
        im = im.copy()
        im.thumbnail((420, h - 40))
        canvas = Image.new("RGB", (440, h), "white")
        canvas.paste(im, ((440 - im.width)//2, 30 + (h - 40 - im.height)//2))
        return canvas
    left = fit(left)
    right = fit(right)
    canvas = Image.new("RGB", (900, h + 20), "white")
    canvas.paste(left, (0, 10))
    canvas.paste(right, (450, 10))
    d = ImageDraw.Draw(canvas)
    d.text((10, 5), "CADe_SIMU 4.0", fill="black")
    d.text((460, 5), "FioVivo reconstruction", fill="black")
    d.text((450, h - 6), ours_type, fill="gray")
    canvas.save(os.path.join(out, ours_type + ".png"))
    print("wrote", ours_type)
