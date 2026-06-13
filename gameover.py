from variables import WHITE

def has_legal_moves(game_board, color):
    for r in range(8):
        for c in range(8):
            piece = game_board.grid[r][c]
            if piece and piece.color == color:
                moves = game_board.get_safe_legal_moves(r, c)
                if len(moves) > 0:
                    return True
    return False


def check_game_over_states(game_board):
    current_turn = game_board.turn
    in_check = game_board.is_in_check(current_turn)
    has_moves = has_legal_moves(game_board, current_turn)

    if in_check and not has_moves:
        return True, f"Checkmate! {'Black' if current_turn == WHITE else 'White'} wins!"

    if not in_check and not has_moves:
        return True, "Draw by Stalemate!"

    if is_insufficient_material(game_board.grid):
        return True, "Draw by Insufficient Material!"

    if getattr(game_board, "halfmove_clock", 0) >= 100:
        return True, "Draw by 50-Move Rule!"

    if check_threefold_repetition(game_board):
        return True, "Draw by Threefold Repetition!"

    return False, ""


def is_insufficient_material(grid):
    active_pieces = []
    for r in range(8):
        for c in range(8):
            piece = grid[r][c]
            if piece is not None:
                active_pieces.append(piece)

    if len(active_pieces) == 2:
        return True

    if len(active_pieces) == 3:
        for p in active_pieces:
            if p.type in (3, 4):
                return True

    if len(active_pieces) == 4:
        white_bishops = []
        black_bishops = []
        for p in active_pieces:
            if p.type == 3:
                if p.color == WHITE:
                    white_bishops.append((p, (r, c)))
                else:
                    black_bishops.append((p, (r, c)))
                    
        if len(white_bishops) == 1 and len(black_bishops) == 1:
            wb_row, wb_col = white_bishops[0][1]
            bb_row, bb_col = black_bishops[0][1]
            if (wb_row + wb_col) % 2 == (bb_row + bb_col) % 2:
                return True

    return False

def check_threefold_repetition(game_board):
    if not hasattr(game_board, "move_history") or len(game_board.move_history) < 5:
        return False

    def get_strict_signature(grid_matrix, turn, ep_target):
        sig = []
        for r in range(8):
            for c in range(8):
                cell = grid_matrix[r][c]
                if cell is None:
                    sig.append(".")
                elif isinstance(cell, dict):
                    sig.append(f"{cell.get('type')}{cell.get('color')}")
                else:
                    sig.append(f"{getattr(cell, 'type')}{getattr(cell, 'color')}")
        
        sig.append(f"|t:{turn}")
        sig.append(f"|ep:{ep_target if ep_target else '-'}")
        
        castling = ""
        for r, c, side in [(7, 4, "WK"), (7, 0, "WQ"), (0, 4, "BK"), (0, 0, "BQ")]:
            piece = grid_matrix[r][c]
            if piece and not isinstance(piece, dict) and not getattr(piece, "has_moved", False):
                castling += side
            elif piece and isinstance(piece, dict) and not piece.get("has_moved", False):
                castling += side
        sig.append(f"|c:{castling if castling else '-'}")
        
        return "".join(sig)

    signature_counts = {}

    for history_item in game_board.move_history:
        if not isinstance(history_item, dict) or "grid" not in history_item:
            continue
            
        hist_grid = history_item["grid"]
        hist_turn = history_item.get("turn", None)
        hist_ep = history_item.get("en_passant_target", None)
        
        sig = get_strict_signature(hist_grid, hist_turn, hist_ep)
        
        signature_counts[sig] = signature_counts.get(sig, 0) + 1
        if signature_counts[sig] >= 3:
            return True

    return False