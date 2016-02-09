CONFIG -= qt
TEMPLATE = aux

spin.target = jm_hackable_ebadge.binary
spin.commands = openspin jm_hackable_ebadge.spin
spin.depends = $$files(*.spin)

PRE_TARGETDEPS = jm_hackable_ebadge.binary
QMAKE_EXTRA_TARGETS = spin
