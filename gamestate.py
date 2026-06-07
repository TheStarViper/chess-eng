EMPTY_PIECE = 0
PAWN = 1
KNIGHT = 2
BISHOP = 3
ROOK = 4
QUEEN = 5
KING = 6

WHITE = "W"
BLACK = "B"

class ChessPiece:
    def __init__(self, piece_type, color):
        self.type = piece_type
        self.color = color

class ChessBoard:
    def __init__(self):
        self.grid = [[None for _ in range(8)] for _ in range(8)]
        self.turn = WHITE
        self.captured_white = []
        self.captured_black = []
        self.initialize_standard_board()

    def initialize_standard_board(self):
        back_rank_setup = [ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK]
        for column in range(8):
            self.grid[0][column] = ChessPiece(back_rank_setup[column], BLACK)
            self.grid[1][column] = ChessPiece(PAWN, BLACK)
            self.grid[6][column] = ChessPiece(PAWN, WHITE)
            self.grid[7][column] = ChessPiece(back_rank_setup[column], WHITE)

    def get_legal_moves(self, start_row, start_column):
        legal_moves = []
        piece = self.grid[start_row][start_column]
        if not piece: 
            return legal_moves

        if piece.type == PAWN:
            direction = -1 if piece.color == WHITE else 1
            start_rank = 6 if piece.color == WHITE else 1
            
            next_row = start_row + direction
            if 0 <= next_row < 8 and self.grid[next_row][start_column] is None:
                legal_moves.append((next_row, start_column))
                
                double_row = start_row + (2 * direction)
                if start_row == start_rank and self.grid[double_row][start_column] is None:
                    legal_moves.append((double_row, start_column))
            
            for column_offset in [-1, 1]:
                target_column = start_column + column_offset
                if 0 <= next_row < 8 and 0 <= target_column < 8:
                    target_square = self.grid[next_row][target_column]
                    if target_square and target_square.color != piece.color:
                        legal_moves.append((next_row, target_column))

        elif piece.type == KNIGHT:
            knight_offsets = [
                (-2, -1), (-2, 1), (-1, -2), (-1, 2),
                (1, -2), (1, 2), (2, -1), (2, 1)
            ]
            for row_offset, col_offset in knight_offsets:
                target_row = start_row + row_offset
                target_column = start_column + col_offset
                if 0 <= target_row < 8 and 0 <= target_column < 8:
                    target_square = self.grid[target_row][target_column]
                    if target_square is None or target_square.color != piece.color:
                        legal_moves.append((target_row, target_column))

        elif piece.type == BISHOP or piece.type == ROOK or piece.type == QUEEN:
            directions = []
            if piece.type == BISHOP or piece.type == QUEEN:
                directions.extend([(-1, -1), (-1, 1), (1, -1), (1, 1)])
            if piece.type == ROOK or piece.type == QUEEN:
                directions.extend([(-1, 0), (1, 0), (0, -1), (0, 1)])
                
            for row_offset, col_offset in directions:
                current_row = start_row + row_offset
                current_column = start_column + col_offset
                
                while 0 <= current_row < 8 and 0 <= current_column < 8:
                    target_square = self.grid[current_row][current_column]
                    
                    if target_square is None:
                        legal_moves.append((current_row, current_column))
                    elif target_square.color != piece.color:
                        legal_moves.append((current_row, current_column))
                        break
                    else:
                        break
                        
                    current_row += row_offset
                    current_column += col_offset

        elif piece.type == KING:
            king_offsets = [
                (-1, -1), (-1, 0), (-1, 1),
                (0, -1),           (0, 1),
                (1, -1),  (1, 0),  (1, 1)
            ]
            for row_offset, col_offset in king_offsets:
                target_row = start_row + row_offset
                target_column = start_column + col_offset
                if 0 <= target_row < 8 and 0 <= target_column < 8:
                    target_square = self.grid[target_row][target_column]
                    if target_square is None or target_square.color != piece.color:
                        legal_moves.append((target_row, target_column))
                    
        return legal_moves

    def make_move(self, start_position, end_position):
        start_row, start_column = start_position
        end_row, end_column = end_position
        
        # Check if there is a piece being captured
        target_piece = self.grid[end_row][end_column]
        if target_piece is not None:
            if target_piece.color == WHITE:
                self.captured_white.append(target_piece)
            else:
                self.captured_black.append(target_piece)
        
        self.grid[end_row][end_column] = self.grid[start_row][start_column]
        self.grid[start_row][start_column] = None
        self.turn = BLACK if self.turn == WHITE else WHITE