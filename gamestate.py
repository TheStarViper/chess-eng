import copy
from variables import DEBUGMODE
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
        self.has_moved = False #track for castling

class ChessBoard:
    def __init__(self):
        self.grid = [[None for _ in range(8)] for _ in range(8)] 
        self.turn = WHITE
        self.captured_white = []
        self.captured_black = []
        self.last_move = None 
        self.promotion_required = False
        self.en_passant_target = None # Will store a tuple (row, column) of the square that can be captured en passant, or None if not applicable
        self.promotion_square = (None, None) # Will store a tuple (row, column)
        self.game_over_reason = None  
        self.halfmove_clock = 0 
        self.game_over_reason = None
        self.position_history = {}  # Tracks Fen-like string states for 3-fold
        self.record_current_position() # Log the starting layout
        self.move_history = []  # List of dictionaries holding snapshots
        self.record_history_state("Start") # Record initial state
        self._move_cache = None
        self._last_zobrist_hash = None 

        self.initialize_standard_board()
    def initialize_standard_board(self):
        back_rank_setup = [ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK]
        for column in range(8):
            self.grid[0][column] = ChessPiece(back_rank_setup[column], BLACK)
            self.grid[1][column] = ChessPiece(PAWN, BLACK)
            self.grid[6][column] = ChessPiece(PAWN, WHITE)
            self.grid[7][column] = ChessPiece(back_rank_setup[column], WHITE)

    def get_legal_moves(self, start_row, start_column, ignore_castling=False):
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

            if self.en_passant_target is not None:
                target_row, target_column = self.en_passant_target
                if next_row == target_row and abs(start_column - target_column) == 1:
                    legal_moves.append((target_row, target_column))

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

        elif piece.type in (BISHOP, ROOK, QUEEN):
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

            if not ignore_castling:
                if not piece.has_moved and not self.is_in_check(piece.color):
                    # Short castle
                    kingside_rook = self.grid[start_row][7]
                    if kingside_rook and kingside_rook.type == ROOK and not kingside_rook.has_moved:
                        if self.grid[start_row][5] is None and self.grid[start_row][6] is None:
                            if not self.is_square_attacked(start_row, 5, piece.color):
                                legal_moves.append((start_row, 6))

                    # Long castle
                    queenside_rook = self.grid[start_row][0]
                    if queenside_rook and queenside_rook.type == ROOK and not queenside_rook.has_moved:
                        if self.grid[start_row][1] is None and self.grid[start_row][2] is None and self.grid[start_row][3] is None:
                            if not self.is_square_attacked(start_row, 3, piece.color):
                                legal_moves.append((start_row, 2))
                                    
        return legal_moves

    def get_safe_legal_moves(self, start_row, start_column):
        if DEBUGMODE:
            print(f"[DEBUG] get_safe_legal_moves called for ({start_row}, {start_column})")
        piece = self.grid[start_row][start_column]
        if not piece or piece.color != self.turn:
            return []

        raw_moves = self.get_legal_moves(start_row, start_column)
        safe_moves = []

        king_row, king_col = self.find_king_position(piece.color)
        in_check_init = self.is_in_check(piece.color)

        is_aligned_with_king = False
        if piece.type != KING:
            row_diff = start_row - king_row
            col_diff = start_column - king_col
            is_aligned_with_king = (row_diff == 0 or col_diff == 0 or abs(row_diff) == abs(col_diff))

        for target_row, target_column in raw_moves:
            if not in_check_init and piece.type != KING and not is_aligned_with_king:
                safe_moves.append((target_row, target_column))
                continue

            original_destination_piece = self.grid[target_row][target_column]

            self.grid[target_row][target_column] = piece
            self.grid[start_row][start_column] = None

            if not self.is_in_check(piece.color):
                safe_moves.append((target_row, target_column))

            self.grid[start_row][start_column] = piece
            self.grid[target_row][target_column] = original_destination_piece

        return safe_moves
    
    def make_move(self, start_position, end_position):
        start_row, start_column = start_position
        end_row, end_column = end_position
        reset_clock = False
        
        self.record_current_position()
        self.check_three_fold()
        
        moving_piece = self.grid[start_row][start_column]
        if not moving_piece:
            return
            
        was_capture = self.grid[end_row][end_column] is not None
        self.record_history_state(start_position, end_position, moving_piece, was_capture)
        
        target_piece = self.grid[end_row][end_column]
        if target_piece is not None:
            if target_piece.color == WHITE:
                self.captured_white.append(target_piece)
            else:
                self.captured_black.append(target_piece)
                
        self.grid[end_row][end_column] = self.grid[start_row][start_column]
        self.grid[start_row][start_column] = None
        
        if moving_piece and moving_piece.type == KING:
            if end_column - start_column == 2:
                rook = self.grid[end_row][7]
                self.grid[end_row][5] = rook
                self.grid[end_row][7] = None
                if rook: rook.has_moved = True
            elif start_column - end_column == 2:
                rook = self.grid[end_row][0]
                self.grid[end_row][3] = rook
                self.grid[end_row][0] = None
                if rook: rook.has_moved = True

        self.en_passant_target = None
        if moving_piece and moving_piece.type == PAWN:
            if abs(start_row - end_row) == 2:
                skipped_row = (start_row + end_row) // 2
                self.en_passant_target = (skipped_row, start_column)

        if moving_piece:
            moving_piece.has_moved = True

        if moving_piece and moving_piece.type == PAWN:
            reset_clock = True
            if end_row == 0 or end_row == 7:
                self.promotion_required = True
                self.promotion_square = (end_row, end_column)
                self.last_move = (start_position, end_position)
                return  
            
        if reset_clock:
            self.halfmove_clock = 0
        else:
            self.halfmove_clock += 1
        
        if self.halfmove_clock >= 100:
            self.game_over_reason = "FIFTY_MOVE_RULE"
            
        self.turn = BLACK if self.turn == WHITE else WHITE
        self.last_move = (start_position, end_position)

        self.evaluate_game_over_conditions(self.turn)

    def find_king_position(self, king_color):
        for row in range(8):
            for column in range(8):
                piece = self.grid[row][column]
                if piece and piece.type == KING and piece.color == king_color:
                    return (row, column)
        return None

    def is_in_check(self, king_color):
        king_position = self.find_king_position(king_color)
        if not king_position:
            return False

        enemy_color = BLACK if king_color == WHITE else WHITE

        for row in range(8):
            for column in range(8):
                piece = self.grid[row][column]
                if piece and piece.color == enemy_color:
                    
                    enemy_moves = self.get_raw_moves(row, column)
                    if king_position in enemy_moves:
                        return True
        return False 
    

    def get_raw_moves(self, row, column):
        return self.get_legal_moves(row, column, ignore_castling=True)
    
    def is_square_attacked(self, row, column, friendly_color):
        enemy_color = BLACK if friendly_color == WHITE else WHITE
        for board_row in range(8):
            for board_column in range(8):
                enemy_piece = self.grid[board_row][board_column]
                if enemy_piece and enemy_piece.color == enemy_color:
                    if (row, column) in self.get_raw_moves(board_row, board_column):
                        return True
        return False

    def promote_pawn(self, choice_type):
        if not self.promotion_required or not self.promotion_square:
            return
            
        row, column = self.promotion_square
        self.grid[row][column] = ChessPiece(choice_type, self.turn)
        
        self.promotion_required = False
        self.promotion_square = None
        self.turn = BLACK if self.turn == WHITE else WHITE

        self.evaluate_game_over_conditions(self.turn)

    def is_checkmate(self, color):
        original_turn = self.turn
        self.turn = color
        
        total_legal_moves = 0
        for row in range(8):
            for column in range(8):
                piece = self.grid[row][column]
                if piece and piece.color == color:
                    safe_moves = self.get_safe_legal_moves(row, column)
                    total_legal_moves += len(safe_moves)
                    
        self.turn = original_turn

        if total_legal_moves > 0:
            return False

        if self.is_in_check(color):
            self.game_over_reason = "CHECKMATE"
            return True
        return False

    def is_stalemate(self, color):
        original_turn = self.turn
        self.turn = color
        
        total_legal_moves = 0
        for row in range(8):
            for column in range(8):
                piece = self.grid[row][column]
                if piece and piece.color == color:
                    safe_moves = self.get_safe_legal_moves(row, column)
                    total_legal_moves += len(safe_moves)
                    
        self.turn = original_turn

        if total_legal_moves == 0 and not self.is_in_check(color):
            self.game_over_reason = "STALEMATE"
            return True
        return False

    def check_three_fold(self):
        for state_key, count in self.position_history.items():
            if count >= 3:
                self.game_over_reason = "REPETITION"
                return True
        return False

    def is_insufficient_material(self):
        white_pieces = []
        black_pieces = []
        for r in range(8):
            for c in range(8):
                p = self.grid[r][c]
                if p:
                    if p.color == WHITE:
                        white_pieces.append(p.type)
                    else:
                        black_pieces.append(p.type)
                        
        all_pieces = white_pieces + black_pieces
        if 1 in all_pieces or 4 in all_pieces or 5 in all_pieces:
            return False
            
        if len(all_pieces) <= 3:
            self.game_over_reason = "INSUFFICIENT_MATERIAL"
            return True
        return False

    def resign(self, losing_color):
        self.game_over_reason = "WHITE_RESIGN" if losing_color == WHITE else "BLACK_RESIGN"

    def declare_draw(self):
        self.game_over_reason = "DRAW"

    def game_over(self):
        if self.game_over_reason is not None:
            return True
            
        if self.is_checkmate(self.turn) or self.is_stalemate(self.turn) or self.is_insufficient_material():
            return True
            
        return False
    
    def generate_position_string(self):
        state_parts = []
        for row in range(8):
            for col in range(8):
                piece = self.grid[row][col]
                if piece is None:
                    state_parts.append(".")
                else:
                    state_parts.append(f"{piece.color}_{piece.type}")
        
        state_parts.append(self.turn)
        return "|".join(state_parts)

    

    def get_square_notation(self, row, col):
        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        ranks = ['8', '7', '6', '5', '4', '3', '2', '1']
        return f"{files[col]}{ranks[row]}"

    def record_history_state(self, start_pos=None, end_pos=None, moving_piece=None, is_capture=False):
        grid_snapshot = []
        for row in range(8):
            row_data = []
            for col in range(8):
                piece = self.grid[row][col]
                if piece is None:
                    row_data.append(None)
                else:
                    row_data.append({"type": piece.type, "color": piece.color})
            grid_snapshot.append(row_data)

        notation = "Start"
        if start_pos and end_pos and moving_piece:
            dest_square = self.get_square_notation(end_pos[0], end_pos[1])
            if moving_piece.type == 6 and abs(start_pos[1] - end_pos[1]) == 2:
                notation = "O-O" if end_pos[1] == 6 else "O-O-O"
            else:
                mapping = {2: "N", 3: "B", 4: "R", 5: "Q", 6: "K"}
                prefix = mapping.get(moving_piece.type, "")
                if is_capture:
                    if moving_piece.type == 1:
                        files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
                        prefix = files[start_pos[1]]
                    prefix += "x"
                notation = f"{prefix}{dest_square}"

        snapshot = {
            "grid": grid_snapshot,
            "turn": self.turn,
            "last_move": self.last_move,
            "notation": notation
        }
        self.move_history.append(snapshot)


    def record_current_position(self):
        current_state = []
        for row in range(8):
            row_state = []
            for col in range(8):
                piece = self.grid[row][col]
                if piece is not None:
                    row_state.append((piece.type, piece.color))
                else:
                    row_state.append(None)
            current_state.append(tuple(row_state))
            
        position_key = (tuple(current_state), self.turn, self.en_passant_target)
        
        # Update dictionary safely
        if position_key in self.position_history:
            self.position_history[position_key] += 1
        else:
            self.position_history[position_key] = 1
    
    def get_king_position(grid, color_target):

        for row in range(8):
            for col in range(8):
                piece = grid[row][col]
                if piece and piece.type == KING and piece.color == color_target:
                    return (row, col)
        return None