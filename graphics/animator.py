import pygame

class PieceAnimation:
    def __init__(self, piece_type, color, start_pos, end_pos, tile_size, board_offset, speed=0.15):

        self.piece_type = piece_type
        self.color = color
        self.speed = speed
        self.is_active = True
        
        self.current_x = board_offset[0] + (start_pos[1] * tile_size) + (tile_size // 2)
        self.current_y = board_offset[1] + (start_pos[0] * tile_size) + (tile_size // 2)
        
        self.target_x = board_offset[0] + (end_pos[1] * tile_size) + (tile_size // 2)
        self.target_y = board_offset[1] + (end_pos[0] * tile_size) + (tile_size // 2)

    def update(self):
        if not self.is_active:
            return

        dx = self.target_x - self.current_x
        dy = self.target_y - self.current_y

        self.current_x += dx * self.speed
        self.current_y += dy * self.speed

        if abs(self.target_x - self.current_x) < 1 and abs(self.target_y - self.current_y) < 1:
            self.current_x = self.target_x
            self.current_y = self.target_y
            self.is_active = False