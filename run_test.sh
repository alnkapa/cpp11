nc localhost 5000 <<EOF
INSERT A 0 lean
INSERT A 1 sweater
INSERT A 2 frank
INSERT A 3 violation
INSERT A 4 quality
INSERT A 5 precision

INSERT B 3 proposal
INSERT B 4 example
INSERT B 5 lake
INSERT B 6 flour
INSERT B 7 wonder
INSERT B 8 selection

INTERSECTION

SYMMETRIC_DIFFERENCE

TRUNCATE A
TRUNCATE B
EOF
