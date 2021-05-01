from manim import *

class BitwiseOperatorOR(Scene):
    def construct(self):
        # Initial - Source code
        ah = Tex("AH = 0xE")
        al = Tex("AL = 'a'")
        currentTitle = Tex("Source code", color='#FFFF00')
        ah.shift(UP)
        currentTitle.shift(DOWN)

        self.play(FadeIn(ah),
            FadeIn(al),
            FadeIn(currentTitle))
        self.wait(1)

        # Compiled
        self.play(Transform(al, Tex("AL = 0x61 (ASCII)")),
            Transform(currentTitle, Tex("Compiled", color='#FFFF00').shift(DOWN)))
        self.wait(1)

        # Binary conversion
        self.play(Transform(ah, Tex("AH = 0000 1110").shift(UP*0.8)),
            Transform(al, Tex("AL = 0110 0001")),
            Transform(currentTitle, Tex("Binary", color='#FFFF00').shift(DOWN)))
        self.wait(1)

        # Shifted AH
        ah.generate_target()
        ah.target.shift(LEFT*1.17)
        ahtail = Tex("0000 0000")
        self.play(MoveToTarget(ah),
            FadeIn(ahtail.shift(UP*0.78).shift(RIGHT*1.85)),
            Transform(currentTitle, Tex("Shifted", color='#FFFF00').shift(DOWN)))
        self.wait(1)

        # Bitwise OR operation
        # Bit lazy to implementing proper procedural
        ax = Tex("AX = ").shift(LEFT*2.35)
        axbin = [Integer(0).next_to(ax, RIGHT*0.9)]
        axbinaryLongDistanceScale = 0.86
        axbinaryShortDistanceScale = 0.15
        for i in range(15):
            if (i+1) % 4 != 0:
                axbin.append(Integer(0).next_to(axbin[i], RIGHT*axbinaryShortDistanceScale))
            else:
                axbin.append(Integer(0).next_to(axbin[i], RIGHT*axbinaryLongDistanceScale))
        bar = Line(LEFT*3, RIGHT*3, stroke_opacity=0.7).shift(UP*0.5)
        ahtail.generate_target()
        currentTitle.generate_target()
        ah.target.shift(UP)
        ahtail.target.shift(UP)
        currentTitle.target.shift(DOWN)
        self.play(MoveToTarget(ah),
            MoveToTarget(ahtail),
            Transform(al, Tex("AL = 0000 0000 0110 0001").shift(UP)),
            MoveToTarget(currentTitle),
            FadeIn(ax),
            FadeIn(axbin[0]), # Berry sad
            FadeIn(axbin[1]),
            FadeIn(axbin[2]),
            FadeIn(axbin[3]),
            FadeIn(axbin[4]),
            FadeIn(axbin[5]),
            FadeIn(axbin[6]),
            FadeIn(axbin[7]),
            FadeIn(axbin[8]),
            FadeIn(axbin[9]),
            FadeIn(axbin[10]),
            FadeIn(axbin[11]),
            FadeIn(axbin[12]),
            FadeIn(axbin[13]),
            FadeIn(axbin[14]),
            FadeIn(axbin[15]),
            FadeIn(bar),
            Transform(currentTitle, Tex("Bitwise OR", color='#FFFF00').shift(DOWN)))
        self.wait(1)

        operatorRect = Rectangle(height=2.4, width=0.35, color=YELLOW)
        operatorRect.set_fill(YELLOW, opacity=0.1).shift(UP*0.87).shift(RIGHT*2.835)
        self.play(FadeInFrom(operatorRect, RIGHT))

        operatorRect.generate_target()
        for i in range(4):
            for j in range(3):
                # Berry berry sad
                if i == 0 and j == 0:
                    self.play(Transform(axbin[15], Integer(1).next_to(axbin[14], RIGHT*axbinaryShortDistanceScale)))
                elif i == 1 and j == 1:
                    self.play(Transform(axbin[10], Integer(1).next_to(axbin[9], RIGHT*axbinaryShortDistanceScale)))
                elif i == 1 and j == 2:
                    self.play(Transform(axbin[9], Integer(1).next_to(axbin[8], RIGHT*axbinaryShortDistanceScale)))
                elif i == 2 and j == 1:
                    self.play(Transform(axbin[6], Integer(1).next_to(axbin[5], RIGHT*axbinaryShortDistanceScale)))
                elif i == 2 and j == 2:
                    self.play(Transform(axbin[5], Integer(1).next_to(axbin[4], RIGHT*axbinaryShortDistanceScale)))
                operatorRect.target.shift(LEFT*0.25)
                self.play(MoveToTarget(operatorRect))
                self.wait(0.05)

            if i < 3:
                if i == 2:
                    self.play(Transform(axbin[4], Integer(1).next_to(axbin[3], RIGHT*axbinaryLongDistanceScale)))
                operatorRect.target.shift(LEFT*0.41)
                self.play(MoveToTarget(operatorRect))
                self.wait(0.05)

        self.play(FadeOut(operatorRect))
        self.wait(1)

        # Hexadecimal conversion
        self.play(FadeOut(ah),
            FadeOut(al),
            FadeOut(ahtail),
            FadeOut(bar))

        ax.generate_target()
        ax.target.shift(UP*1.5)
        for binnumber in axbin:
            binnumber.generate_target()
            binnumber.target.shift(UP*1.5)

        hexarrow = [Vector(direction=DOWN, color='#FFFF00') for i in range(4)]
        hexarrow[0].shift(LEFT*1.05).shift(UP*1.21).scale(0.7)
        hexarrow[1].shift(RIGHT*0.1).shift(UP*1.21).scale(0.7)
        hexarrow[2].shift(RIGHT*1.3).shift(UP*1.21).scale(0.7)
        hexarrow[3].shift(RIGHT*2.5).shift(UP*1.21).scale(0.7)

        hexax = Tex("AX = ").shift(LEFT*2.35)

        hexDistanceScale = 0.71
        axhexval = [Tex("0").next_to(hexax).shift(RIGHT*0.28)]
        axhexval.append(Tex("E").next_to(axhexval[0]).shift(RIGHT*hexDistanceScale))
        axhexval.append(Tex("6").next_to(axhexval[1]).shift(RIGHT*hexDistanceScale))
        axhexval.append(Tex("1").next_to(axhexval[2]).shift(RIGHT*hexDistanceScale))

        self.play(MoveToTarget(ax),
            MoveToTarget(axbin[0]),
            MoveToTarget(axbin[1]),
            MoveToTarget(axbin[2]),
            MoveToTarget(axbin[3]),
            MoveToTarget(axbin[4]),
            MoveToTarget(axbin[5]),
            MoveToTarget(axbin[6]),
            MoveToTarget(axbin[7]),
            MoveToTarget(axbin[8]),
            MoveToTarget(axbin[9]),
            MoveToTarget(axbin[10]),
            MoveToTarget(axbin[11]),
            MoveToTarget(axbin[12]),
            MoveToTarget(axbin[13]),
            MoveToTarget(axbin[14]),
            MoveToTarget(axbin[15]),

            FadeIn(hexax),

            Transform(currentTitle, Tex("Hexadecimal", color='#FFFF00').shift(DOWN)))

        for i in range(3, -1, -1):
            self.play(FadeIn(hexarrow[i]),
            FadeIn(axhexval[i]))
            self.wait(0.05)

        hexax.generate_target()
        hexax.target.shift(UP+RIGHT)
        hexNotation = Tex("0x").next_to(hexax.target).shift(RIGHT*0.05)
        axhexval[0].generate_target()
        axhexval[0].target.next_to(hexNotation).shift(LEFT*0.19)
        for i in range(1,4):
            axhexval[i].generate_target()
            axhexval[i].target.next_to(axhexval[i-1].target)

        axhexval[1].target.shift(LEFT*0.2)
        axhexval[3].target.shift(LEFT*0.2)

        ahunder = Tex("$\\underbrace{ { } {} }_{AH}$").shift(RIGHT*0.09+UP*0.3)
        alunder = Tex("$\\underbrace{ { } }_{AL}$").shift(RIGHT*1.33+UP*0.3)

        self.play(
            FadeOut(hexarrow[0]),
            FadeOut(hexarrow[1]),
            FadeOut(hexarrow[2]),
            FadeOut(hexarrow[3]),

            FadeOut(ax),
            FadeOut(axbin[0]), # Berry sad
            FadeOut(axbin[1]),
            FadeOut(axbin[2]),
            FadeOut(axbin[3]),
            FadeOut(axbin[4]),
            FadeOut(axbin[5]),
            FadeOut(axbin[6]),
            FadeOut(axbin[7]),
            FadeOut(axbin[8]),
            FadeOut(axbin[9]),
            FadeOut(axbin[10]),
            FadeOut(axbin[11]),
            FadeOut(axbin[12]),
            FadeOut(axbin[13]),
            FadeOut(axbin[14]),
            FadeOut(axbin[15]),

            FadeIn(hexNotation),
            FadeIn(ahunder),
            FadeIn(alunder),
            MoveToTarget(hexax),
            MoveToTarget(axhexval[0]),
            MoveToTarget(axhexval[1]),
            MoveToTarget(axhexval[2]),
            MoveToTarget(axhexval[3]))

        self.wait(4)
        self.play(FadeOut(currentTitle),
            FadeOut(axhexval[0]),
            FadeOut(axhexval[1]),
            FadeOut(axhexval[2]),
            FadeOut(axhexval[3]),

            FadeOut(hexax),
            FadeOut(hexNotation),
            FadeOut(ahunder),
            FadeOut(alunder))
